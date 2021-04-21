# Homework 1 - Multimedia

<br>Scrivere un programma in C (UTF8_encoding) 
che converta dal file input.data (che contiene caratteri scritti in UCS-4)
al file UTF8.data, che deve contenere gli stessi caratteri scritti in UTF-8.</br>
<br>Per fare questo leggere a gruppi di 4 byte da input.data e convertire in UTF-8 esteso a 7 byte.</br>

<br>Scrivere poi un programma (UTF8_decoding) che converta i caratteri da UTF-8 esteso a 7 byte
a UCS-4</br>

____

**Comandi per la compilazione e il controllo della correttezza del risultato**

```
gcc UTF8_encoding.c -o encoder && ./encoder && diff UTF8.data check_UTF8.data
gcc UTF8_decoding.c -o decoder && ./decoder && diff output.data input.data
```

**Resources**

* https://www.rapidtables.com/convert/number/hex-to-binary.html
* https://it.wikipedia.org/wiki/Universal_Character_Set

