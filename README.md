<<<<<<< HEAD
# GomuluSistemler_Odev3_STM32F103
STM32F103C8T6 Bluepill kartında TIM2 kesmesiyle LED kontrolü, PA0 buton girişi ve Flash bellekte sayaç saklama uygulaması.
## Proje Özeti

- TIM2, 1 saniyede bir kesme oluşturacak şekilde ayarlanmıştır.
- PC13 üzerindeki dahili LED, belirlenen sayaç değeri kadar yanıp söner.
- PA0 pini buton girişi olarak kullanılmıştır.
- PA1 pini çıkış olarak ayarlanmış ve lojik 0 seviyesinde tutulmuştur.
- Sayaç değeri 4 ile 7 arasında değişmektedir.
- Sayaç değeri değiştiğinde dahili Flash belleğe kaydedilir.
- Sistem yeniden başlatıldığında sayaç değeri Flash bellekten okunur.
- Açılışta buton 3 saniye basılı tutulursa sistem varsayılan değere döner.
=======
# BSM316 Gömülü Sistemler Ödev 3

Bu projede STM32F103C8T6 Bluepill kartı kullanılarak TIM2 interrupt ile PC13 üzerindeki dahili LED kontrol edilmiştir.

## Özellikler

- TIM2 saniyede 1 kez interrupt üretir.
- PC13 LED, `blink_count` değeri kadar yanıp söner.
- LED yanıp sönme döngüsünden sonra 5 saniye sönük bekler.
- PA0 buton girişi olarak kullanılmıştır.
- PA1 GPIO output olarak ayarlanmıştır ve program boyunca lojik 0 kalır.
- `blink_count` değeri 4 ile 7 arasında değişir.
- `blink_count` her değiştiğinde STM32 dahili Flash belleğine kaydedilir.
- Açılışta `blink_count` Flash bellekten okunur.
- Açılışta PA0 butonu en az 3 saniye basılı tutulursa fabrika ayarına dönülür ve `blink_count = 4` olur.
>>>>>>> 45a3a91 (Initial commit)

## Kullanılan Donanım

- STM32F103C8T6 Bluepill
<<<<<<< HEAD
- ST-Link programlayıcı
- PC13 dahili LED
- PA0 buton girişi
- PA1 lojik 0 çıkışı
=======
- ST-Link
- PA0-A1 arası buton bağlantısı veya kısa devre teması
- PC13 dahili LED

## Video

Video linki: buraya YouTube linkini yaz
>>>>>>> 45a3a91 (Initial commit)
# GomuluSistemler_Odev3_STM32F103Devresi
