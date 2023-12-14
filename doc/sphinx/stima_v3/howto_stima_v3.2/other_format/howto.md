# STAZIONI RMAP


| Versione | Autore             | Data       |
| -------- | ------------------ | ---------- |
| 1.0      | Alessio Valgimigli | 03/11/2023 |
|          |                    |            |
|          |                    |            |
|          |                    |            |



[TOC]

##  Stazioni di monitoraggio

Le stazioni oggetto della fornitura consentono la lettura , memorizzazione e invio dei seguenti sensori:

- Anemometro GILL WINDOSONIC OPTION1
- Sensore Temperatura umidità basato sul sensore SHT35
- Pluviometro a bascula da 0.2mm
- Radiometro 0-1V 0-2000W/m2
- Batteria e pannello solare

La soluzione prevista è quella mostrata in figura:

 ![](./Immagine/totale.png)

Cioè si compone di 6 moduli che seguono: 

- Stima
- i2c-th
- i2c-Power
- i2c-radiation
- i2c-wind
- i2c-rain

### Stima

In figuralo schema di massima della pila Stima con jumper e configurazione:

![](./immagine/Stima.png)

### i2c-th

In figura lo schema di massima della pila i2c-th con jumper e configurazione:

![](./immagine/i2c-th.png)

### i2c-power

In figura lo schema di massima della pila i2c-power con jumper e configurazione:

![](./immagine/i2c-power.png)

In particolare per la scheda ADC è prevista una modifica con taglio pista e la sostituzione delle resistenza R3 e R7 da 20k a 8.2k.



### i2c-radiation

In figura lo schema di massima della pila i2c-radiation con jumper e configurazione:

![](./immagine/i2c-radiation.png)

### i2c-wind

In figura lo schema di massima della pila i2c-wind con jumper e configurazione:

![](./immagine/i2c-wind.png)

### i2c-rain

In figura lo schema di massima della pila i2c-wind con jumper e configurazione:

![](./immagine/i2c-rain.png)

## Schema di collegamento 

In figura è riportato lo schema di collegamento del quadro:

![](./immagine/schema.png)

Segue invece schema dettaglio della morsettiera di confine:

![](./immagine/morsettiera.png)
