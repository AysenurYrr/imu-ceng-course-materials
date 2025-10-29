// Global değişkenler
let ws = null;
let connected = false;
let drones = {};
let survivors = {
    waiting: [],
    helped: []
};
let previousDronePositions = {}; // Drone'ların önceki konumlarını saklamak için
let lastDroneId = 0; // En son kullanılan drone ID'si
let isLaunchingDrones = false; // Drone başlatma işleminin durumunu takip etmek için
let launchQueue = []; // Başlatılacak drone'ların kuyruğu
let launchInterval = null; // Drone başlatma aralığı

// DOM elemanları
const connectionStatus = document.getElementById('connection-status');
const droneList = document.getElementById('drone-list');
const mapView = document.getElementById('map-view');
const droneCountValue = document.getElementById('drone-count-value');
const waitingSurvivors = document.getElementById('waiting-survivors');
const helpedSurvivors = document.getElementById('helped-survivors');
const launchButton = document.getElementById('launch-drones');

// Harita boyutları - globals.h ile senkronize
const MAP_HEIGHT = 40;
const MAP_WIDTH = 30;

// Bildirim sistemi
function showNotification(message, type = 'info') {
    // Bildirim container'ını oluştur
    const notificationContainer = document.createElement('div');
    notificationContainer.className = `notification ${type}`;
    
    // Bildirim içeriğini oluştur
    const notificationContent = document.createElement('div');
    notificationContent.className = 'notification-content';
    notificationContent.textContent = message;
    
    // Kapatma butonunu oluştur
    const closeButton = document.createElement('button');
    closeButton.className = 'notification-close';
    closeButton.innerHTML = '&times;';
    closeButton.onclick = () => {
        notificationContainer.classList.add('fade-out');
        setTimeout(() => notificationContainer.remove(), 300);
    };
    
    // Bildirim elementlerini birleştir
    notificationContainer.appendChild(notificationContent);
    notificationContainer.appendChild(closeButton);
    
    // Bildirimi sayfaya ekle
    document.body.appendChild(notificationContainer);
    
    // Otomatik kapanma
    setTimeout(() => {
        if (notificationContainer.parentElement) {
            notificationContainer.classList.add('fade-out');
            setTimeout(() => notificationContainer.remove(), 300);
        }
    }, 5000);
}

// WebSocket bağlantısını başlat
function connectWebSocket() {
    // WebSocket URL'sini oluştur
    const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
    const wsUrl = `${protocol}//${window.location.hostname}:8081`;
    
    // WebSocket bağlantısını oluştur
    ws = new WebSocket(wsUrl);
    
    // WebSocket event listener'ları
    ws.onopen = () => {
        console.log('WebSocket bağlantısı açıldı');
        connectionStatus.textContent = 'Sunucu ile bağlantı kuruldu';
        connectionStatus.className = 'connected';
        connected = true;
        showNotification('Sunucu ile bağlantı kuruldu', 'success');
        
        // Bağlantı kurulduğunda kuyrukta bekleyen drone'ları başlat
        if (launchQueue.length > 0) {
            processLaunchQueue();
        }
    };
    
    ws.onclose = () => {
        console.log('WebSocket bağlantısı kapandı');
        connectionStatus.textContent = 'Sunucu ile bağlantı kesildi. Yeniden bağlanılıyor...';
        connectionStatus.className = 'disconnected';
        connected = false;
        showNotification('Sunucu ile bağlantı kesildi. Yeniden bağlanılıyor...', 'error');
        
        // Drone başlatma işlemini durdur
        isLaunchingDrones = false;
        if (launchInterval) {
            clearInterval(launchInterval);
            launchInterval = null;
        }
        
        // 5 saniye sonra yeniden bağlanmayı dene
        setTimeout(connectWebSocket, 5000);
    };
    
    ws.onerror = (error) => {
        console.error('WebSocket hatası:', error);
        connectionStatus.textContent = 'Bağlantı hatası!';
        connectionStatus.className = 'disconnected';
        showNotification('Bağlantı hatası oluştu!', 'error');
    };
    
    ws.onmessage = (event) => {
        try {
            const data = JSON.parse(event.data);
            processMessage(data);
        } catch (error) {
            console.error('Gelen mesaj işlenirken hata oluştu:', error);
        }
    };
}

// WebSocket'ten gelen mesajları işle
function processMessage(data) {
    if (data.type === 'status_update') {
        // İstatistikleri güncelle
        updateStatistics(data);
        
        // Drone listesini güncelle
        updateDroneList(data.drones);
        
        // Haritayı güncelle
        updateMap(data.drones, data.survivors);
    }
}

// İstatistikleri güncelle
function updateStatistics(data) {
    // Drone sayısını güncelle
    const droneCount = data.drones ? data.drones.length : 0;
    droneCountValue.textContent = droneCount;
    
    // Survivor sayılarını güncelle
    if (data.survivors) {
        let waitingCount = 0;
        let helpedCount = 0;
        
        data.survivors.forEach(survivor => {
            if (survivor.status === 'waiting') {
                waitingCount++;
            } else if (survivor.status === 'helped') {
                helpedCount++;
            }
        });
        
        waitingSurvivors.textContent = waitingCount;
        helpedSurvivors.textContent = helpedCount;
    }
}

// Drone listesini güncelle
function updateDroneList(droneData) {
    if (!droneData || droneData.length === 0) {
        droneList.innerHTML = '<div class="no-drones">Henüz aktif drone bulunmuyor.</div>';
        return;
    }
    
    // Drone'ları sakla
    drones = {};
    droneData.forEach(drone => {
        drones[drone.id] = drone;
        
        // En yüksek drone ID'sini takip et
        const droneIdNum = parseInt(drone.id);
        if (!isNaN(droneIdNum) && droneIdNum > lastDroneId) {
            lastDroneId = droneIdNum;
            // Son drone ID'sini localStorage'a kaydet
            localStorage.setItem('lastDroneId', lastDroneId);
        }
    });
    
    // Drone listesini temizle
    droneList.innerHTML = '';
    
    // Drone template'ini al
    const template = document.getElementById('drone-item-template');
    
    // Her drone için listeye eleman ekle
    droneData.forEach(drone => {
        const droneItem = template.content.cloneNode(true);
        
        droneItem.querySelector('.drone-id').textContent = `Drone #${drone.id}`;
        
        const statusElement = droneItem.querySelector('.drone-status');
        statusElement.textContent = `Durum: ${drone.status === 'idle' ? 'Boşta' : 'Görevde'}`;
        statusElement.classList.add(drone.status);
        
        // SDL ile uyumlu hale getirmek için koordinatları düzelterek göster
        const x = drone.position.x;
        const y = drone.position.y;
        droneItem.querySelector('.drone-position').textContent = `Konum: (${x}, ${y})`;
        
        droneList.appendChild(droneItem);
    });
}

// Haritayı güncelle
function updateMap(droneData, survivorData) {
    // Haritayı temizle, ama drone'ları silme
    const existingDrones = {};
    const droneElements = mapView.querySelectorAll('.map-drone');
    droneElements.forEach(element => {
        const id = element.getAttribute('data-id');
        if (id) {
            existingDrones[id] = element;
            // Şimdilik elementleri DOM'dan kaldırma
            mapView.removeChild(element);
        }
    });
    
    // Haritayı temizle
    mapView.innerHTML = '';
    
    // Harita ölçeklendirme faktörlerini hesapla
    const scaleX = mapView.offsetWidth / MAP_WIDTH;
    const scaleY = mapView.offsetHeight / MAP_HEIGHT;
    
    // Izgara çizgilerini çiz
    drawGrid(scaleX, scaleY);
    
    // Her hücre için bir arka plan oluştur
    for (let y = 0; y < MAP_HEIGHT; y++) {
        for (let x = 0; x < MAP_WIDTH; x++) {
            const cell = document.createElement('div');
            cell.className = 'map-cell';
            cell.style.left = `${x * scaleX}px`;
            cell.style.top = `${y * scaleY}px`;
            cell.style.width = `${scaleX}px`;
            cell.style.height = `${scaleY}px`;
            mapView.appendChild(cell);
        }
    }
    
    // survivorleri haritaya ekle
    if (survivorData && survivorData.length > 0) {
        survivorData.forEach(survivor => {
            // Sadece 'waiting' durumundaki survivor'ları haritaya ekle
            if (survivor.status === 'waiting') {
                const survivorElement = document.createElement('div');
                survivorElement.className = `map-item map-survivor ${survivor.status}`;
                
                // SDL ile uyumlu hale getirmek için x ve y koordinatlarını değiştir
                const x = survivor.position.y; // SDL'de y, web'de x olur
                const y = survivor.position.x; // SDL'de x, web'de y olur
                
                survivorElement.style.left = `${(x + 0.5) * scaleX}px`;
                survivorElement.style.top = `${(y + 0.5) * scaleY}px`;
                survivorElement.title = `survivor #${survivor.id}`;
                
                mapView.appendChild(survivorElement);
            }
        });
    }
    
    // Drone'ları haritaya ekle
    if (droneData && droneData.length > 0) {
        droneData.forEach(drone => {
            let droneElement;
            
            // Eğer drone zaten haritada varsa, konumunu güncelle
            if (existingDrones[drone.id]) {
                droneElement = existingDrones[drone.id];
            } else {
                // Yoksa yeni bir element oluştur
                droneElement = document.createElement('div');
                droneElement.className = `map-item map-drone ${drone.status}`;
                droneElement.setAttribute('data-id', drone.id);
                droneElement.title = `Drone #${drone.id}`;
            }
            
            // SDL ile uyumlu hale getirmek için x ve y koordinatlarını değiştir
            const x = drone.position.y; // SDL'de y, web'de x olur
            const y = drone.position.x; // SDL'de x, web'de y olur
            const targetX = drone.target.y; // SDL'de target.y, web'de targetX olur
            const targetY = drone.target.x; // SDL'de target.x, web'de targetY olur
            
            droneElement.style.left = `${(x + 0.5) * scaleX}px`;
            droneElement.style.top = `${(y + 0.5) * scaleY}px`;
            droneElement.className = `map-item map-drone ${drone.status}`;
            
            mapView.appendChild(droneElement);
            
            // Görevdeki drone'lar için hedef çizgisi çiz
            if (drone.status === 'busy') {
                drawTargetLine(
                    { x: (x + 0.5) * scaleX, y: (y + 0.5) * scaleY },
                    { x: (targetX + 0.5) * scaleX, y: (targetY + 0.5) * scaleY },
                    scaleX, scaleY
                );
            }
        });
    }
}

// Izgara çizgilerini çiz
function drawGrid(scaleX, scaleY) {
    const gridContainer = document.createElement('div');
    gridContainer.className = 'grid-container';
    
    // Yatay çizgiler
    for (let y = 0; y <= MAP_HEIGHT; y++) {
        const horizontalLine = document.createElement('div');
        horizontalLine.className = 'grid-line horizontal';
        horizontalLine.style.top = `${y * scaleY}px`;
        horizontalLine.style.width = `${MAP_WIDTH * scaleX}px`;
        gridContainer.appendChild(horizontalLine);
    }
    
    // Dikey çizgiler
    for (let x = 0; x <= MAP_WIDTH; x++) {
        const verticalLine = document.createElement('div');
        verticalLine.className = 'grid-line vertical';
        verticalLine.style.left = `${x * scaleX}px`;
        verticalLine.style.height = `${MAP_HEIGHT * scaleY}px`;
        gridContainer.appendChild(verticalLine);
    }
    
    mapView.appendChild(gridContainer);
}

// Hedef çizgisi çiz
function drawTargetLine(start, end, scaleX, scaleY) {
    const line = document.createElement('div');
    line.className = 'target-line';
    
    // Başlangıç ve bitiş noktaları
    const x1 = start.x;
    const y1 = start.y;
    const x2 = end.x;
    const y2 = end.y;
    
    // Çizgi uzunluğu
    const length = Math.sqrt(Math.pow(x2 - x1, 2) + Math.pow(y2 - y1, 2));
    
    // Çizgi açısı
    const angle = Math.atan2(y2 - y1, x2 - x1) * 180 / Math.PI;
    
    // Çizgiyi konumlandır ve döndür
    line.style.width = `${length}px`;
    line.style.left = `${x1}px`;
    line.style.top = `${y1}px`;
    line.style.transform = `rotate(${angle}deg)`;
    line.style.transformOrigin = '0 0';
    
    mapView.appendChild(line);
}

// Drone başlatma kuyruğunu işle
function processLaunchQueue() {
    if (!connected || launchQueue.length === 0) {
        isLaunchingDrones = false;
        if (launchInterval) {
            clearInterval(launchInterval);
            launchInterval = null;
        }
        return;
    }

    const message = launchQueue.shift();
    try {
        ws.send(JSON.stringify(message));
        console.log(`Drone #${message.drone_id} başlatılıyor...`);
    } catch (error) {
        console.error('Drone başlatma hatası:', error);
        // Hata durumunda mesajı kuyruğun başına geri ekle
        launchQueue.unshift(message);
    }
}

// Yeni drone başlat
function launchDrones() {
    if (!connected) {
        showNotification('Sunucu ile bağlantı kurulamadı! Lütfen sunucunun çalıştığından emin olun.', 'error');
        return;
    }
    
    if (isLaunchingDrones) {
        showNotification('Drone başlatma işlemi devam ediyor. Lütfen bekleyin.', 'warning');
        return;
    }
    
    const serverIp = document.getElementById('server-ip').value;
    const serverPort = parseInt(document.getElementById('server-port').value);
    const droneCount = parseInt(document.getElementById('drone-count').value);
    
    // Başlangıç ID'sini localStorage'dan al veya input'tan al
    let startId = parseInt(document.getElementById('start-id').value);
    
    // Eğer input'ta değer varsa, localStorage'daki değeri güncelle
    if (!isNaN(startId) && startId > 0) {
        localStorage.setItem('lastDroneId', startId - 1);
    } else {
        // Eğer input'ta değer yoksa, localStorage'dan al veya varsayılan değer kullan
        const savedLastId = localStorage.getItem('lastDroneId');
        startId = savedLastId ? parseInt(savedLastId) + 1 : 1;
        document.getElementById('start-id').value = startId;
    }
    
    if (!serverIp || isNaN(serverPort) || isNaN(droneCount) || isNaN(startId)) {
        showNotification('Lütfen tüm alanları doğru şekilde doldurun.', 'error');
        return;
    }
    
    if (droneCount <= 0 || droneCount > 50) {
        showNotification('Drone sayısı 1 ile 50 arasında olmalıdır.', 'error');
        return;
    }
    
    if (startId <= 0) {
        showNotification('Başlangıç ID değeri pozitif olmalıdır.', 'error');
        return;
    }
    
    // Drone başlatma kuyruğunu oluştur
    launchQueue = [];
    for (let i = 0; i < droneCount; i++) {
        const droneId = (startId + i).toString();
        launchQueue.push({
            type: 'launch_drone',
            drone_id: droneId,
            server_ip: serverIp,
            server_port: serverPort
        });
    }
    
    // Drone başlatma işlemini başlat
    isLaunchingDrones = true;
    processLaunchQueue();
    
    // Her 500ms'de bir drone başlat (bağlantıyı yormamak için)
    launchInterval = setInterval(processLaunchQueue, 500);
    
    // Son drone ID'sini güncelle
    lastDroneId = startId + droneCount - 1;
    localStorage.setItem('lastDroneId', lastDroneId);
    
    // Bir sonraki drone için başlangıç ID'sini güncelle
    document.getElementById('start-id').value = lastDroneId + 1;
    
    showNotification(`${droneCount} adet drone başlatma isteği kuyruğa eklendi.`, 'success');
}

// Sayfa yüklendiğinde
document.addEventListener('DOMContentLoaded', () => {
    // WebSocket bağlantısını başlat
    connectWebSocket();
    
    // Son drone ID'sini localStorage'dan al
    const savedLastId = localStorage.getItem('lastDroneId');
    if (savedLastId) {
        lastDroneId = parseInt(savedLastId);
        // Başlangıç ID'sini son ID + 1 olarak ayarla
        document.getElementById('start-id').value = lastDroneId + 1;
    }
    
    // Drone başlatma butonuna tıklama eventi ekle
    launchButton.addEventListener('click', launchDrones);
}); 