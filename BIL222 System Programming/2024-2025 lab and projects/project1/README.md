# 🧑‍💻 Proje Üyeleri

- Ayşenur Yörür
- Sevban Bozaslan
- Hüseyin Melih Özbek
---

![Uygulama Tanıtımı](assets/output.gif)

# 🐚 GTK Tabanlı Çoklu Kabuk Uygulaması (Multi Shell Interface)

Bu proje, C dili ve GTK+3 kütüphanesi kullanılarak geliştirilmiş, birden fazla terminal kabuğunu aynı anda çalıştırabileceğiniz **grafiksel kullanıcı arayüzüne** sahip bir shell simülatörüdür.  
Kullanıcı, başlangıç ekranında kaç kabuk penceresi açmak istediğini belirler ve her pencere terminal komutları ve mesajlaşma özellikleriyle gelir.

---

## 🚀 Özellikler

- ✅ Çoklu terminal penceresi
- ✅ Komut geçmişi (↑ ↓ tuşları ile erişim)
- ✅ Gerçek zamanlı terminal çıktısı
- ✅ Paylaşılan mesaj alanı
- ✅ `@msg` ile tüm kabuklara ortak paylaşım alanına mesaj gönderme
- ✅ `@msgX` (örneğin `@msg2 Merhaba`) komutu ile sadece belirli bir kabuğa özel mesaj gönderme (DM)
- ✅ Başlangıç ekranında kaç tane kabuk oluşturulacağının seçimi

### 🔒 Özel Mesajlaşma (Direct Message - DM)

- `@msgX` formatı ile belirli bir shell'e özel mesaj gönderebilirsiniz.
- Buradaki `X`, mesajın gönderileceği terminal penceresinin **numarasıdır**.
- Örnek:
  - `@msg2 Selam!` komutu sadece **Shell 2** penceresinde görünür.
  - Diğer kabuklarda bu mesaj **görünmez**.
- `@msg` şeklinde boşlukla yazarsanız mesaj **tüm pencerelere** gönderilir.

💡 Arka planda, her mesajın hedef kabuk ID’si shared memory üzerinden kontrol edilir. Sadece o ID'ye sahip shell, mesajı görüntüler.
- Her mesaj `shared memory (paylaşımlı bellek)` içinde bir `hedef shell ID` bilgisiyle birlikte saklanır.
- Shell ID `0` ise mesaj herkese açıktır.
- Shell ID `X` ise yalnızca o ID’ye sahip terminal bu mesajı görür.
- Diğer terminaller, bu mesajı okuyamaz.

## 🗂️ Dosya Açıklamaları Design (MVC Architecture)

Bu proje, **Model-View-Controller (MVC)** mimarisine uygun olarak tasarlanmıştır. Her bileşenin sorumlulukları ayrıdır ve kodun okunabilirliği ile sürdürülebilirliğini artırır.

| Dosya Adı       | Açıklama |
|-----------------|----------|
| `main.c`        | Uygulamanın giriş noktası. Başlangıç ekranı burada oluşturulur. |
| `controller.c`  | Kullanıcının girişini kontrol eder. Mesaj mı komut mu olduğunu ayırt eder ve uygun işlemi başlatır. |
| `view.c`        | Tüm GTK arayüz öğelerinin (pencereler, etiketler, giriş alanı vb.) oluşturulması ve güncellenmesinden sorumludur. |
| `model.c`       | Komut yürütme, mesaj gönderme ve paylaşılan bellek işlemlerini içerir. |
| `common.h`      | Yapılar, sabitler ve fonksiyon bildirileri bu başlık dosyasında yer alır. Ayrıca global değişkenler içerir. |


### 📦 Model (`model.c`)

- Terminal komutlarının yürütülmesinden sorumludur.
- `fork()`, `execvp()`, `pipe()` ve `select()` gibi sistem çağrılarını kullanarak komut çıktısını yakalar.
- Paylaşılan belleğe (`shm_open`, `mmap`) erişir.
- Mesajları okur ve yazar (örn: `model_send_message`, `model_read_messages`).
- `@msg` gibi mesajlar için formatlı metinleri oluşturur.

### 🎨 View (`view.c`)

- GTK arayüzünü yönetir.
- Her kabuk için pencere, terminal görünümü, mesaj görünümü ve giriş kutusu oluşturur.
- Terminal ve mesaj görünümlerini günceller (`view_update_terminal`, `view_update_message_area`).
- Kullanıcıdan komut girildiğinde `controller`'a sinyal gönderir.
- Komut geçmişi navigasyonu sağlar (↑ ↓ tuşları).

### 🧠 Controller (`controller.c`)

- Kullanıcı girişlerini işler.
- `@msg` gibi mesaj komutlarını ayıklar ve `model` fonksiyonlarını çağırır.
- Normal komutları algılar ve `model_execute_command` ile yürütülmesini sağlar.
- Tüm kabuklara güncel mesajları düzenli olarak iletir (`controller_update_views`).
- Güncelleme zamanlayıcısını (`g_timeout_add`) yönetir.

### 🏁 Main (`main.c`)

- Programın başlangıç noktasıdır.
- GTK başlatılır, başlangıç penceresi (`show_startup_window`) gösterilir.
- Kabuk sayısı seçildikten sonra `view_init` ve `controller_init` çağrılır.
- Uygulama kapatıldığında kaynaklar temizlenir.

---

Bu yapı sayesinde projenin mantıksal katmanları birbirinden ayrılır ve bakım yapılması kolaylaşır.

---

## Temizle ve Derleme

```bash
make clean
make
```


## ▶️ Çalıştırma

```bash
./multi_shell
```

Program çalıştığında küçük bir pencere açılır ve kaç kabuk penceresi başlatmak istediğinizi seçebilirsiniz.

---

## 💬 Mesajlaşma Kullanımı

- Ortak mesaj alanına mesaj yazmak için:

```bash
@msg Merhaba herkese!
```

- Dm ile private mesaj atma:

```bash
@msg2 Sadece Shell 2'ye selam veriyorum!

```

```bash
@msg3 Sadece Shell 3'e selam veriyorum!

```

- Komut çalıştırmak örnekleri:

```bash
ls -l
mkdir selam
rm -rf selam
cd ..
```

---

## Temizlik

Program kapandığında tüm GTK pencereleri, paylaşımlı bellek ve semaforlar temizlenir.

---

## 📌 Gereksinimler

- `gtk+-3.0` kütüphanesi (Linux)
- POSIX uyumlu sistem
