README.md
# BİL 304 İşletim Sistemleri Projesi Raporu
24060839 CEMİLE YERLİKAYA
23060563 ZEYNEP SUDE YÜKSEL

Sistem isterlerine uygun olarak çekilen, projenin çalışma anını, düğümler arası trafiği ve kullanılan hash algoritmasının teorik açıklamasını içeren video bağlantısı aşağıdadır:


https://www.youtube.com/watch?v=61974PxzoPA


 1. Geliştirme İş Parçacığı: Sistem Mimarisi ve Yöntemler

Bu projede, Nesnelerin İnterneti (IoT) ağlarında yaygın olarak kullanılan Düşük Güçlü ve Kayıplı Ağlar için Yönlendirme Protokolü (**RPL**) mimarisi, **Contiki-NG** işletim sistemi ve **Cooja** simülatörü kullanılarak test edilmiştir.

### Ağ Topolojisi ve Düğümler (Nodes)
* **1 Adet Server (Kök Düğüm / Sink Node):** Ağın merkezinde yer alır. Diğer düğümlerden gelen UDP paketlerini toplar och yönlendirme ağacının kökü olarak görev yapar.
* **5 Adet Client (İstemci Düğüm / Sender Node):** Çevreye dağıtılmış durumdadır. Periyodik olarak sensör verisi simüle ederek Server düğümüne UDP paketleri gönderirler.

### Kullanılan Yöntemler ve Protokol Detayları
* **Protokol:** `examples/rpl-udp` dizini altında yer alan RPL tabanlı UDP haberleşme mekanizması temel alınmıştır.
* **Yönlendirme (Routing):** RPL protokolü, düğümler arasında dinamik olarak bir yönlendirme ağacı oluşturur.
* **Veri Akışı:** İstemciler (Client 1-5), Server düğümünün IPv6 adresini RPL mekanizması üzerinden öğrenir ve hedef odaklı UDP paket gönderimi gerçekleştirir.

---

## 📦 2. Paket Uzunlukları, Analizler ve Alınan Önlemler

### Paket Yapısı ve Uzunlukları
Ağ üzerinde iletilen standart mesaj paketlerinin boyutları ve içerikleri Contiki-NG standart yapısına göre optimize edilmiştir:
* **Uygulama Katmanı Verisi (Payload):** Simüle edilen sensör verisini barındırır (Ortalama 10-20 byte).
* **Protokol Ekleri (Overhead):** IPv6 başlığı (40 byte) ve UDP başlığı (8 byte) paket üzerine eklenir.
* **Toplam Paket Boyutu:** IEEE 802.15.4 standartlarının izin verdiği maksimum fiziksel paket boyutu (MTU = 127 byte) sınırları aşılmayacak şekilde, verimlilik adına paket boyutu küçük ve optimize tutulmuştur.

### Alınan Önlemler ve Kararlılık
1. **Paket Kayıplarının Önlenmesi:** Kayıplı kablosuz kanallardaki paket düşmelerini en aza indirmek için Cooja üzerinde düğümlerin konumları dengeli yerleştirilmiştir.
2. **Radio Traffic Gözlemi:** Simülasyon sırasında *Radio Traffic* ve *Timeline* araçları aktif olarak izlenmiş, düğümlerin aynı anda paket göndererek çarpışmaya (collision) sebep olması engellenmiştir.

---

## 🔒 3. Hash Algoritmaları Teorik Bilgisi (Video İsteri)

Projeye ait video anlatımında da değinildiği üzere, IoT sistemlerinin güvenliğinde veri bütünlüğünü sağlamak adına **Hash (Özet) Algoritmaları** kritik bir öneme sahiptir.

* **Hash Nedir?:** Hash algoritmaları, boyutu ne olursa olsun herhangi bir girdi verisini alıp, sabit uzunlukta benzersiz bir karakter dizisine (özete) dönüştüren tek yönlü matematiksel fonksiyonlardır. 
* **IoT Dağıtımlarında Tercih Sebebi:** Şifrelemeden farklı olarak hash işlemlerinin geri dönüşü yoktur. Gönderilen verinin yolda değiştirilip değiştirilmediğini (Bütünlük) doğrulamak için harika bir yöntemdir.
* **SHA-256 Standartı:** IoT cihazlarının kısıtlı kaynakları göz önünde bulundurulduğunda, hem yüksek güvenlik sunan hem de optimize donanımsal ivmelendiricilerle desteklenen **SHA-256** algoritması bütünlük kontrolleri için endüstri standardı olarak kabul edilir.
