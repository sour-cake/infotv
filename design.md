# InfoTV-hallintaohjelma

## Arkkitehtuuri

### Palvelin
Palvelin on ohjelma joka lähettää komentoja näytölle TLS TCP:n kautta.
Palvelin lukee lähetettäviä komentoja STDIN:stä.
Määritys tehdään JSON-tiedoston kautta.

### Näyttö
Näyttö on Raspberry PI -tietokone, joka on yhdistettynä näyttöön.
Näyttö ottaa yhteyden palvelimeen TLS TCP:n kautta, ja vastaa palvelimen komentoihin.
Määritys tehdään JSON-tiedoston kautta.

### Sisältö
Näytettävä sisältö on lähtökohtaisesti muuttumattomia kuvia.
Uusia kuvia voi luoda joka päivälle esim. skripteillä.
Sisältö synkronoidaan rsync-sovelluksella.

### Aikataulu
Näytöillä on oma aikataulu, joka on JSON-tiedosto.
Aikataulun tiedostonimi on `_schedule.json`.
Aikataulu synkronisoidaan kuvasisällön mukana.

### Yhteys
Palvelimen IP-osoite täytyy olla näkyvissä näytölle.
Palvelimella täytyy olla tiedossa näytön public-key.
Palvelimen ja näytön välinen kommunikointi tapahtuu TCP:n kautta.
Yhteys uudelleekäynnistetään yhden kerran joka vuorokausi järjestyslukujen kierrättämiseksi.

### Komennot ja vastaukset (viestit)
Viestit ovat UTF-8 -tekstirivejä joita erottaa 1+ rivinvaihto (0x0A).
UTF-8 koodausvirheestä täytyy ilmoittaa.
Viestin oletetaan olevan kokonainen vasta kun rivinvaihtokirjain on vastaanotettu.
Jokaisen viestin alussa on heksadesimaalinen järjestysluku, joka alkaa laskemaan luvusta 1.
Komennot kulkevat palvelimelta näytölle, ja vastaukset kulkevat näytöltä palvelimelle.

## Ohjelmisto

### Komennot

#### RSYNC-komento
Näyttö suorittaa rsync-ohjelman jolla sisältö synkronoidaan palvelimen kanssa.

`<id> rsync`

#### URGENT-komento
Vaihtaa näytön kuvan välittömästi.

`<id> urgent "<filename>"`
- `<filename>` kuvan tiedostonimi

#### ENDURGENT-komento
Näyttö lopettaa välittömän kuvan näyttämisen ja jatkaa aikataulun mukaisesti.

`<id> endurgent`

### Vastaukset

#### ERROR-vastaus
Ilmoittaa virheestä palvelimelle.

`<id> error <cause> "<description>"`
- `<cause>` Virheen aiheuttaneen komennon järjestysluku, tai 0 jos ei komennon aiheuttama.
- `<description>` kuvaus virheestä.

### Aikataulu

Päivä-aikamuoto:
	Päivämäärät ja ajat annetaan kirjain-numero yhdistelminä, esim. M12 on joulukuu.
	Aikavälin voi antaa väliviivaa käyttämällä, esim. P1-5 on arkipäivät.
	Viikonpäivät alkavat maanantaista.
	Y=vuosi, Mkuukausi, V=viikko, D=kuukaudenpäivä, P=viikonpäivä,
	h=tunti, m=minuutti, s=sekuntti.

Aikataulun muoto on seuraava:
- `slot_time` kertoo kuinka kauan yhtä kuvaa pidetään näytöllä.
- `slots` on lista näytettävistä kuvista.
- `slots[].name` on kuvan tiedostonimi.
- `slots[].date` on päivä(t) jolle kuva rajoittuu.
	Jos joku kirjain puuttuu, se tarkoittaa ettei rajoituksia ole sen tasolla.

```
{
	"slot_time" : "m1",
	"slots" : [
		{ "name" : "slide1.png" },
		{
			"date" : "M12D1-24",
			"name" : "hyvää-joulua.png"
		},
		{
			"date" : "P5",
			"name" : "hyvää-viikonloppua.png"
		}
	]
}
```

### Käytettävät kirjastot

#### Server
- OpenSSL 3.0.x (Päivitettävä 2026-09-07 kun long-term-support loppuu)
- JSON-C

#### Client
- OpenSSL 3.0.x (Päivitettävä 2026-09-07 kun long-term-support loppuu)
- JSON-C
- SDL 2.0.x
- SDL_Image 2.0.x
