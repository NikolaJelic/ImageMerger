

## Optimizacija algoritma - Stapanje slika uzimanjem prosječne ili maksimalne vrijednosti piksela



## Opis projekta

Opšti cilj ovog izveštaja je uporediti četiri verzije implementacije algoritma za spajanje slika. Prva verzija je osnovna verzija koja koristi neučinkovitu ugnježđenu petlju za prolazak kroz matricu, što dovodi do loše upotrebe keša. Druga verzija je optimizovana za keš, koristeći jednodimenzionalni niz za prolazak kroz matricu, što poboljšava upotrebu keša. Treća verzija koristi OpenMP sa pravilno napisanom ugnježdenom petljom iz osnovne funkcije, što omogućava paralelizaciju i bolje korišćenje višejezgarnih procesora. Četvrta verzija je kombinacija keš i OpenMP optimizacija.

Za svaku verziju algoritma merenje performansi obavljeno je pomoću alata cachegrind, a učinkovitost implementacije analizirana je u smislu vremena izvršavanja i keš pogodaka. 



Mjerenja se vrše sa i bez kompajlerskih optimizacija, koje su za kompajler `clang`  - **o0** i  **ofast.**

**o0**  pruža najbrže kompajliranje koda i najpogodniji je za debagovanje ali su sve optimizacije isključene.

**ofast**  uvodi sve standardne optimizacije sa dodatnim agresivnim optimizacijama koje mogu krštiti pravila jezičkog standarda ili dovesti do nedefinisanog ponašanja



## Algoritam

U ovom projektu se vrši analiza algoritma za stapanje slika uzimanjem prosječne ili maksimalne vrijednosti piksela.

Algoritam prima sljedeće parametre:

- Vrstu spajanja: spajanje po srednjoj ili maksimalnoj vrijednosti
- Putanju do prve ulazne slike
- Putanju do druge ulazne slike
- Putanju do izlazne slike
- Težina koja se koristi pri spajanju po srednjoj vrijednosti i mora biti u opsegu [0.0,1.0]



Algoritam prolazi kroz piksele ulaznih slika i kombinuje njihove R, G i B vrijednosti, na osnovu čega generiše novu sliku istih dimenzija kao ulazne.

U slučaju stapanja slika po srednjoj vrijednosti, koristi se sledeća formula za komponente svakog piksela:

```apl
komponenta_izlaznog_piksela = težina * komponenta_piksela_prve_slike + (1- težina) * komponenta_piksela_druge_slike
```

U slučaju stapanja slika po maksimalnoj vrijednosti, koristi se sledeća formula za komponente svakog piksela:

```apl
komponenta_izlaznog_piksela = max(komponenta_piksela_prve_slike, komponenta_piksela_druge_slike)
```

Implementacija algoritma u ovom projektnom zadatku je ograničena na [BMP](https://docs.fileformat.com/image/bmp/) fajlove čiji se svaki piksel sastoji od R, G i B komponenti koje predstavljaju crvenu, zelenu i plavu boju. Svaka komponenta zauzima 8 bita. Osim vrste datoteke, ulazne slike su ograničene i na jednakost dimenzija, tako da ulazne i izlanzna slika svi imaju jednake dimenzije.

Na višem nivou algoritam za stapanje slika se sastoji od sledećih koraka:

```basic
ucitaj_slike
validiraj_slike
za svaki piksel
	primjeni formulu za racunanje nove vrijednosti
sacuvaj novu sliku 
```

### Bazna implementacija

Bazna implementacija ima namjerno degradirane keš performanse pomoću loše implementacije ugniježdene for petlje.
Ova petlja, kao i struktura podataka koja sadrži piksele su elementi koji najviše utiču na performanse u ovoj implementaciji.

```cpp
std::filesystem::path
ImageMerger::merge_images(int merger, const std::filesystem::path &first, const std::filesystem::path &second,
                          const std::filesystem::path &out_path, float weight) {
    try {
        Bmp first_image{first};
        Bmp second_image{second};
		// Provjera da li su slike jednakih dimenzija
        if (first_image.getHeader().height != second_image.getHeader().height ||
            first_image.getHeader().width != second_image.getHeader().width) {
            throw std::runtime_error("Images aren't matching.\n");
        } else {
            // Ucitavanje hedera bmp slike i piksel informaicija
            auto out_header = first_image.getHeader();
            auto const &first_vector = first_image.getPixelData();
            auto const &second_vector = second_image.getPixelData();
            size_t const height = first_image.getHeader().height;
            size_t const width = first_vector.size() / height;
			//konvertovanje vektora sa pikselima u 2d vektor 
            auto first_array = get_2d_pixels(first_vector, height);
            auto second_array = get_2d_pixels(second_vector, height);
			
            std::vector<std::vector<std::byte>> out_array(height, std::vector<std::byte>(width));

            // Obrnut obilazak ugnijezdene for petlje koji znatno pogorsava kes performanse
            for (size_t j = 0; j < width; ++j) {
                for (size_t i = 0; i < height; ++i) {
                    std::byte pixel{};
                    if (merger) {
                        // Formula za srednju vrijednost piksela
                        pixel = std::byte(weight * std::to_integer<int>(first_array[i][j]) +
                                          (1 - weight) * std::to_integer<int>(second_array[i][j]));

                    } else {
                        // Formula za maksimalnu vrijednost piksela
                        pixel = std::byte(std::max(std::to_integer<int>(first_array[i][j]),
                                                   std::to_integer<int>(second_array[i][j])));
                    }
                    out_array[i][j] = pixel;
                }
            }
            auto out_vec = get_vec_pixels(out_array);
            Bmp out{out_header, out_vec};
            // Povratak apsolutne putanje generisane slike
            return absolute(out.write_image(out_path)); 
        }
    } catch (std::exception &e) { std::cerr << e.what(); }
    return {};
}

```



### Keš optimizovana implementacija

Prva optimizacija je korišćenje jednodimenzionalnog niza kontinualnih piksela umesto dvodimenzionalnog niza. Ovo omogućava bolju iskorišćenost keš memorije procesora, što može znatno ubrzati pristup i obradu podataka.

Druga optimizacija je korišćenje operatora `[]` umesto `.at()` za pristupanje elementima niza. Operator `[]` ne vrši provjeru granica i neće izazvati izuzetak u slučaju prekoračenja granica niza što može dovesti do neočekivanih rezultata ili grešaka u programu u slučaju da su indeksi van granica niza. Međutim, korišćenje operatora `[]` umesto `.at()` može blago poboljšati performanse programa, jer ne vrši provjeru granica svaki put kada se pristupa elementu niza.

```cpp
std::filesystem::path
ImageMerger::merge_images_cache(int merger, const std::filesystem::path &first, const std::filesystem::path &second, const std::filesystem::path &out_path, float weight) {
    try {
        Bmp first_image{first};
        Bmp second_image{second};

        if (first_image.getHeader().height != second_image.getHeader().height ||
            first_image.getHeader().width != second_image.getHeader().width) {
            throw std::runtime_error("Images aren't matching.\n");
        } else {
            auto const &first_pixel_data = std::span<const std::byte>(first_image.getPixelData());
            auto const &second_pixel_data = std::span<const std::byte>(second_image.getPixelData());

            auto out_header = first_image.getHeader();
            std::vector<std::byte> out_pixels(first_pixel_data.size());

            // Iteracija se vrši kroz jednodimenzionalni niz kontinualnih piksela sto poboljsava kes performance
            for (size_t i = 0; i < first_pixel_data.size(); ++i) {
            // Pristup pomocu [] umjesto .at() sto blago popravlja performance
                std::byte pixel{};
                if (merger) {
                    pixel = std::byte(weight * std::to_integer<int>(first_pixel_data[i]) +
                                      (1 - weight) * std::to_integer<int>(second_pixel_data[i]));
                } else {
                    pixel = std::byte(std::max(std::to_integer<int>(first_pixel_data[i]),
                                               std::to_integer<int>(second_pixel_data[i])));
                }
                out_pixels[i] = pixel;
            }
            Bmp out{out_header, out_pixels};
            return absolute(out.write_image(out_path));
        }
    } catch (std::exception &e) { std::cerr << e.what(); }
    return {};
}
```



### OpenMP optimizovana implementacija

Za razliku od bazne implementacije, OpenMP implementacija koristi pravilno ugniježdenu for petlju zajedno sa openMP direktivama koje paralelizuju iteriranje kroz petlje. Ovo funkcioniše tako što svaka vanjska i unutrašnja petlja pripadaju različitim oblastima paralelizacije ([izvor](https://www.openmp.org/wp-content/uploads/openmp-examples-4.5.0.pdf#section.9.5))

Spoljna petlja iterira kroz redove slike, a unutrašnja petlja kroz kolone. Za svaki piksel, kod računa spojenu vrednost na osnovu odgovarajućih piksela iz dve ulazne slike i dodeljuje rezultat odgovarajućem pikselu u izlaznoj slici.

Direktiva `#pragma omp parallel` na početku bloka koda kreira paralelnu oblast, gde će se izvršavati kod od strane više niti. Klauzula `default(shared)` označava da će sve promenljive biti deljene između niti, što znači da će svaka nit imati pristup istim kopijama promenljivih.

Direktiva `#pragma omp for` pre svake petlje ukazuje na to da će se petlja paralelizovati pomoću OpenMP-ovog "paralelnog for" konstrukta, koji raspoređuje iteracije petlje među dostupnim nitima. Klauzula `shared` specificira koje promenljive će biti deljene između niti.

Unutar unutrašnje petlje, postoji uslovna naredba koja proverava vrednost bool promenljive `merger`. Ako je ona `true`, vrednost piksela se računa kao težinska sredina odgovarajućih piksela iz dve ulazne slike. Ako je `false`, vrednost piksela se računa kao maksimalna vrednost odgovarajućih piksela iz dve ulazne slike.



```cpp

std::filesystem::path
ImageMerger::merge_images_openmp(int merger, const std::filesystem::path &first, const std::filesystem::path &second,
                                 const std::filesystem::path &out_path, float weight) {
    try {
        Bmp first_image{first};
        Bmp second_image{second};

        if (first_image.getHeader().height != second_image.getHeader().height ||
            first_image.getHeader().width != second_image.getHeader().width) {
            throw std::runtime_error("Images aren't matching.\n");
        } else {
            auto out_header = first_image.getHeader();
            size_t const height = out_header.height;

            auto const &first_pixel_data = get_2d_pixels(first_image.getPixelData(), height);
            auto const &second_pixel_data = get_2d_pixels(second_image.getPixelData(), height);
            size_t const width = first_pixel_data[0].size();
            std::vector<std::vector<std::byte>> out_array(height, std::vector<std::byte>(width));

#pragma omp parallel default(shared)
{
#pragma omp for
            for (size_t i = 0; i < height; ++i) {
#pragma omp parallel shared(i, height)
{
#pragma omp for
                 for (size_t j = 0; j < width; ++j) {
                     std::byte pixel{};
                     if (merger) {
                     pixel = std::byte(weight * std::to_integer<int>(first_pixel_data[i][j]) 										+ (1 - weight) * std::to_integer<int>(second_pixel_data[i][j]));
                  	  } else {
                      pixel = std::byte(std::max(std::to_integer<int>(first_pixel_data[i][j]),
                              std::to_integer<int>(second_pixel_data[i][j])));
                       }
                    out_array[i][j] = pixel;
                 }
}
              }

}
            auto out_vec = get_vec_pixels(out_array);
            Bmp out{out_header, out_vec};
            return absolute(out.write_image(out_path));
        }
    } catch (std::exception &e) { std::cerr << e.what(); }
    return {};
}

```

### Kombinovana optimizacija

Ova implementacija algoritma kombinuje optimizacije iz prethodne dvije implementacije. 
Za stapanje slika iterira se kroz jednodimenzionalni vektor kontinualnih bajtova, a sama petlja je paralelizovana pomoću `#pragma omp parallel for`.

Mjerenja su pokazala da ova implementacija ima ubjedljivo najbolje performanse.

```cpp

std::filesystem::path
ImageMerger::merge_images_optimized(int merger, const std::filesystem::path &first, const std::filesystem::path &second,
                                    const std::filesystem::path &out_path, float weight) {
    try {
        Bmp first_image{first};
        Bmp second_image{second};

        if (first_image.getHeader().height != second_image.getHeader().height ||
            first_image.getHeader().width != second_image.getHeader().width) {
            throw std::runtime_error("Images aren't matching.\n");
        } else {
            auto const &first_pixel_data = first_image.getPixelData();
            auto const &second_pixel_data = second_image.getPixelData();

            auto out_header = first_image.getHeader();
            std::vector<std::byte> out_pixels(first_pixel_data.size());

#pragma omp parallel for
            for (size_t i = 0; i < first_pixel_data.size(); ++i) {
                std::byte pixel{};
                if (merger) {
                    pixel = std::byte(weight * std::to_integer<int>(first_pixel_data[i]) +
                                      (1 - weight) * std::to_integer<int>(second_pixel_data[i]));
                } else {
                    pixel = std::byte(std::max(std::to_integer<int>(first_pixel_data[i]),
                                               std::to_integer<int>(second_pixel_data[i])));
                }
                out_pixels[i] = pixel;
            }
            Bmp out{out_header, out_pixels};
            return absolute(out.write_image(out_path));
        }
    } catch (std::exception &e) { std::cerr << e.what(); }
    return {};
}
```





## Mjerenja

Mjerenja implementacija algoritma vrše se na dva načina - praćenje vremena potrebnog za izvršavanje i pomoću alata cashgrind.

Vrijeme potrebno za izvršavanje funkcije se mjeri pomoću funkcije `*std*::*chrono*::*high_resolution_clock*::*now*();`

```cpp
 auto start = std::chrono::high_resolution_clock::now();
 merger.merge_images(merge_val, first_image, second_image,out_image, weight);
 auto end = std::chrono::high_resolution_clock::now();
 auto elapsed_time = duration_cast<std::chrono::milliseconds>(end - start);
```

Cachegrind je alat za analizu performansi koji mjeri upotrebu keš  memorije tokom izvršavanja programa. On generiše simulaciju keš  memorije i prati upotrebu iste. Na taj način, može se  utvrditi koliko često se pristupa memoriji, koliko je keš promašaja i  koliko memorije se koristi. Cachegrind takođe prati broj izvršenih  instrukcija.

### Rezultati mjerenja alatom cachegrind

Alat `cachegrind` se nad programom pokrece komandom

```shell	
valgrind  --branch-sim=yes --cache-sim=yes --tool=cachegrind ImageMerger implementacija average prva_slika druga_slika izlaz tezina

```



![](https://github.com/NikolaJelic/ImageMerger/blob/main/resources/readme/chart.png)



| Veličina                             | Bazni | Keš  | OpenMP | Optimizovano |
| :----------------------------------- | :---: | :--: | :----: | :----------: |
| Dohvatanje instrukcija               | 6.44  | 5.26 |  6.66  |     5.05     |
| L1  promašaji dohvatanja instrukcija | 0.26  | 0.19 |  0.20  |     0.17     |
| LL  promašaji dohvatanja instrukcija | 0.29  | 0.21 |  0.23  |     0.19     |
| Pristup podacima čitanjem            | 4.84  | 4.57 |  6.19  |     5.38     |
| L1 promašaji čitanja podataka        | 57.05 | 0.89 |  0.89  |     0.89     |
| LL promašaji čitanja podataka        | 17.92 | 5.93 |  5.94  |     5.93     |
| Pristup podacima pisanjem            | 5.78  | 4.89 |  5.78  |     4.44     |
| L1 promašaji pisanja podataka        | 57.55 | 0.90 |  0.90  |     0.90     |
| LL promašaji pisanja podataka        | 8.31  | 2.07 |  1.87  |     1.32     |
| Uvjetni skokovi                      | 5.11  | 5.10 |  5.11  |     5.10     |
| Pogrešni predviđeni uvjetni skokovi  | 4.44  | 0.01 |  1.49  |     0.02     |
| L1 sum promašaja                     | 57.21 | 0.89 |  0.90  |     0.89     |
| Suma promašaja zadnjeg nivoa         | 12.25 | 9.65 |  3.54  |     3.21     |
| Pogrešni predviđeni skokovi          | 4.35  |  0   |  1.47  |     0.02     |
| Procjena ciklusa                     | 9.51  | 4.87 |  6.03  |     4.64     |

Za svaku veličinu manja izmjerena vrijednost predstavlja bolji rezultat.

Bazna implementacija je pokazala daleko najgore rezultate za sljedeće vrijednosti:

- L1 promašaji čitanja podataka
- LL promašaji čitanja podataka  
- L1 promašaji pisanja podataka  
- LL promašaji pisanja podataka  
- Pogrešni predviđeni uvjetni skokovi  
- L1 sum promašaja  

dok su za ostala mjerenja dobijene razlike dosta manje, ali ipak prisutne.

Sve tri optimizovane implementacije algoritma imaju približne izmjerene vrijednosti, pri čemu je kombinovana optimizacija obično najbolja.

OpenMP implementacija smanjuje performance skokova u odnosu na druge dvije  optimizacije, ali je ipak poboljšanje u odnosu na baznu implementaciju.

### Rezultati mjerenja vremena

Rezultati mjerenja su izraženi u nanosekundama.
Zabilježena mjerenja su izvršena nakon što je program pokrenut nekoliko puta radi zagrijavanja.
Sva mjerenja su izvršena na ulazima veličina 256x256, 512x512, 1024x1024 i 2048x2048 piksela i mjerenja za svaku vrstu ulaza su vršena 10 puta. 
Osim razlika u dimenziji ulaza, vršena su mjerenja programa dobijenog sa i bez kompajlerskih optimizacija.

### Ulaz veličine 2048x2048 bez kompajlerskih optimizacija

| Broj mjerenja            |      Bazni      |       Keš       |     OpenMP      | Optimizovani  |
| ------------------------ | :-------------: | :-------------: | :-------------: | :-----------: |
| 1                        |     323.34      |     207.37      |     165.79      |     71.75     |
| 2                        |     304.06      |     274.23      |     161.33      |     78.47     |
| 3                        |     321.08      |     200.64      |     131.93      |     46.53     |
| 4                        |     308.26      |     199.79      |     152.83      |     69.21     |
| 5                        |     345.46      |     192.75      |     154.28      |     79.50     |
| 6                        |     309.84      |     191.61      |     166.40      |     49.08     |
| 7                        |     282.83      |     185.50      |     173.98      |     58.59     |
| 8                        |     321.93      |     202.49      |     140.81      |     51.68     |
| 9                        |     279.26      |     192.40      |     143.68      |     47.56     |
| 10                       |     276.19      |     191.92      |     147.56      |     77.88     |
| Aritmetička sredina      |     307.23      |     203.87      |     153.86      |     63.02     |
| Varijansa                |     498.72      |     653.05      |     172.91      |    188.80     |
| Standardna devijacija    |      22.33      |      25.55      |      13.15      |     13.74     |
| Koeficijent devijacije   |      0.07       |      0.13       |      0.09       |     0.22      |
| c1(90%)                  |     294.28      |     189.06      |     146.24      |     55.06     |
| c2(90%)                  |     320.17      |     218.68      |     161.48      |     70.99     |
| Interval povjerenja(90%) | (294.28,320.17) | (189.06,218.68) | (146.24,161.48) | (55.06,70.99) |
| c1(95%)                  |     291.25      |     185.59      |     144.45      |     53.19     |
| c2(95%)                  |     323.20      |     222.15      |     163.26      |     72.85     |
| Interval povjerenja(95%) | (291.25,323.20) | (185.59,222.15) | (144.45,163.26) | (53.19,72.85) |



### Ulaz veličine 2048x2048 sa kompajlerskim optimizacijama

| Broj mjerenja            |      Bazni       |      Keš       |     OpenMP     |  Optimizovani  |
| ------------------------ | :--------------: | :------------: | :------------: | :------------: |
| 1                        |      188.95      |     68.19      |     60.07      |     32.15      |
| 2                        |      204.19      |     40.01      |     70.26      |     34.74      |
| 3                        |      192.07      |     54.97      |     94.70      |     47.39      |
| 4                        |      188.99      |     60.93      |     60.07      |     32.16      |
| 5                        |      190.18      |     67.67      |     74.46      |     34.55      |
| 6                        |      183.34      |     33.09      |     61.34      |     30.13      |
| 7                        |      183.16      |     56.84      |     69.80      |     32.48      |
| 8                        |      182.74      |     53.41      |     61.56      |     37.41      |
| 9                        |      190.98      |     60.79      |     67.35      |     33.11      |
| 10                       |      220.36      |     38.25      |     76.25      |     36.87      |
| Aritmetička sredina      |      192.50      |     53.42      |     69.59      |     35.10      |
| Varijansa                |      134.63      |     152.16     |     113.09     |     23.65      |
| Standardna devijacija    |      11.60       |     12.34      |     10.63      |      4.86      |
| Koeficijent devijacije   |       0.06       |      0.23      |      0.15      |      0.14      |
| c1(90%)                  |      185.77      |     46.27      |     63.42      |     32.28      |
| c2(90%)                  |      199.22      |     60.57      |     75.75      |     37.92      |
| Interval povjerenja(90%) | (185.77, 199.22) | (46.27, 60.57) | (63.42, 75.75) | (32.28, 37.92) |
| c1(95%)                  |      184.20      |     44.59      |     61.98      |     31.62      |
| c2(95%)                  |      200.80      |     62.24      |     77.19      |     38.58      |
| Interval povjerenja(95%) |  (184.2, 200.8)  | (44.59, 62.24) | (61.98, 77.19) | (31.62, 38.58) |



![](https://github.com/NikolaJelic/ImageMerger/blob/main/resources/readme/2048_graph.png)

### Ulaz veličine 1024x1024 bez kompajlerskih optimizacija

| Broj mjerenja            |     Bazni      |      Keš       |     OpenMP     |  Optimizovani  |
| ------------------------ | :------------: | :------------: | :------------: | :------------: |
| 1                        |     111.51     |     49.34      |     43.76      |     17.17      |
| 2                        |     71.88      |     49.34      |     46.26      |     17.55      |
| 3                        |     69.18      |     46.99      |     45.09      |     16.67      |
| 4                        |     70.76      |     47.85      |     47.56      |     18.43      |
| 5                        |     65.72      |     50.87      |     44.74      |     18.14      |
| 6                        |     70.58      |     50.64      |     55.20      |     19.36      |
| 7                        |     70.23      |     51.75      |     54.93      |     20.86      |
| 8                        |     69.82      |     50.13      |     54.06      |     19.24      |
| 9                        |     71.42      |     51.09      |     56.20      |     19.36      |
| 10                       |     75.97      |     50.05      |     52.62      |     21.95      |
| Aritmetička sredina      |     74.71      |     49.80      |     50.04      |     18.87      |
| Varijansa                |     173.64     |      2.18      |     24.87      |      2.70      |
| Standardna devijacija    |     13.18      |      1.48      |      4.99      |      1.64      |
| Koeficijent devijacije   |      0.18      |      0.03      |      0.10      |      0.09      |
| c1(90%)                  |     67.07      |     48.95      |     47.15      |     17.92      |
| c2(90%)                  |     82.34      |     50.66      |     52.93      |     19.83      |
| Interval povjerenja(90%) | (67.07, 82.34) | (48.95, 50.66) | (47.15, 52.93) | (17.92, 19.83) |
| c1(95%)                  |     65.28      |     48.75      |     46.48      |     17.70      |
| c2(95%)                  |     84.13      |     50.86      |     53.61      |     20.05      |
| Interval povjerenja(95%) | (65.28, 84.13) | (48.75, 50.86) | (46.48, 53.61) | (17.70, 20.05) |



### Ulaz veličine 1024x1024 sa kompajlerskim optimizacijama

| Broj mjerenja            |     Bazni      |      Keš      |     OpenMP     |  Optimizovani  |
| ------------------------ | :------------: | :-----------: | :------------: | :------------: |
| 1                        |     35.67      |     8.96      |     62.32      |     23.80      |
| 2                        |     50.99      |     27.52     |     49.27      |     14.77      |
| 3                        |     47.46      |     8.27      |     44.76      |     16.42      |
| 4                        |     60.47      |     9.47      |     54.56      |     13.73      |
| 5                        |     39.76      |     9.64      |     36.35      |      7.47      |
| 6                        |     29.38      |     9.12      |     20.92      |      7.39      |
| 7                        |     29.46      |     9.35      |     16.02      |     12.86      |
| 8                        |     26.39      |     8.16      |     24.45      |      9.89      |
| 9                        |     28.83      |     12.87     |     20.45      |     12.76      |
| 10                       |     40.78      |     12.31     |     38.94      |     16.53      |
| Aritmetička sredina      |     38.92      |     11.57     |     36.80      |     13.56      |
| Varijansa                |     126.33     |     33.88     |     254.89     |     23.69      |
| Standardna devijacija    |     11.24      |     5.82      |     15.97      |      4.87      |
| Koeficijent devijacije   |      0.29      |     0.50      |      0.43      |      0.36      |
| c1(90%)                  |     32.40      |     8.19      |     27.55      |     10.74      |
| c2(90%)                  |     45.43      |     14.94     |     46.06      |     16.38      |
| Interval povjerenja(90%) | (32.4, 45.43)  | (8.19, 14.94) | (27.55, 46.06) | (10.74, 16.38) |
| c1(95%)                  |     30.88      |     7.40      |     25.38      |     10.08      |
| c2(95%)                  |     46.96      |     15.73     |     48.22      |     17.04      |
| Interval povjerenja(95%) | (30.88, 46.96) | (7.4, 15.73)  | (25.38, 48.22) | (10.08, 17.04) |



![](https://github.com/NikolaJelic/ImageMerger/blob/main/resources/readme/1024_graph.png)

### Ulaz veličine 512x512 bez kompajlerskih optimizacija

| Broj mjerenja            |     Bazni     |      Keš      |    OpenMP    | Optimizovani |
| ------------------------ | :-----------: | :-----------: | :----------: | :----------: |
| 1                        |     44.41     |     16.75     |    10.61     |     4.66     |
| 2                        |     16.76     |     12.28     |    10.26     |     3.86     |
| 3                        |     16.25     |     12.31     |    10.32     |     4.76     |
| 4                        |     18.89     |     12.48     |     7.09     |     3.09     |
| 5                        |     15.21     |     12.65     |    70.10     |     2.64     |
| 6                        |     15.22     |     12.47     |     7.20     |     2.68     |
| 7                        |     15.65     |     12.44     |     6.90     |     2.61     |
| 8                        |     15.27     |     12.46     |     6.88     |     6.04     |
| 9                        |     15.17     |     12.76     |    18.11     |     9.87     |
| 10                       |     31.52     |     12.14     |    13.50     |     2.76     |
| Aritmetička sredina      |     20.43     |     12.87     |    16.10     |     4.30     |
| Varijansa                |     95.88     |     1.89      |    372.71    |     5.19     |
| Standardna devijacija    |     9.79      |     1.37      |    19.31     |     2.28     |
| Koeficijent devijacije   |     0.48      |     0.11      |     1.20     |     0.53     |
| c1(90%)                  |     14.76     |     12.08     |     4.91     |     2.98     |
| c2(90%)                  |     26.11     |     13.67     |    27.29     |     5.62     |
| Interval povjerenja(90%) | (14.76,26.11) | (12.08,13.67) | (4.91,27.29) | (2.98,5.62)  |
| c1(95%)                  |     13.43     |     11.89     |     2.29     |     2.67     |
| c2(95%)                  |     27.44     |     13.86     |    29.91     |     5.93     |
| Interval povjerenja(95%) | (13.43,27.44) | (11.89,13.86) | (2.29,29.91) | (2.67,5.93)  |

### Ulaz veličine 512x512 sa kompajlerskim optimizacijama

| Broj mjerenja            |    Bazni     |     Keš      |    OpenMP    | Optimizovani |
| ------------------------ | :----------: | :----------: | :----------: | :----------: |
| 1                        |    10.87     |     2.37     |     7.03     |     6.27     |
| 2                        |     5.18     |     2.79     |    17.06     |     1.28     |
| 3                        |     5.29     |     1.83     |     2.54     |     1.78     |
| 4                        |     5.25     |     2.73     |     2.95     |     1.59     |
| 5                        |     4.65     |     1.84     |     3.67     |     1.27     |
| 6                        |     4.61     |     1.89     |     2.91     |     1.62     |
| 7                        |     4.16     |     2.08     |     3.05     |     1.32     |
| 8                        |     4.31     |     2.14     |     2.79     |     1.41     |
| 9                        |     4.89     |     2.02     |     2.56     |     1.92     |
| 10                       |     4.24     |     2.08     |     2.68     |     1.61     |
| Aritmetička sredina      |     5.35     |     2.18     |     4.72     |     2.01     |
| Varijansa                |     3.94     |     0.12     |    20.58     |     2.29     |
| Standardna devijacija    |     1.99     |     0.35     |     4.54     |     1.51     |
| Koeficijent devijacije   |     0.37     |     0.16     |     0.96     |     0.75     |
| c1(90%)                  |     4.19     |     1.98     |     2.09     |     1.13     |
| c2(90%)                  |     6.50     |     2.38     |     7.35     |     2.88     |
| Interval povjerenja(90%) | (4.19, 6.5)  | (1.98, 2.38) | (2.09, 7.35) | (1.13, 2.88) |
| c1(95%)                  |     3.92     |     1.93     |     1.48     |     0.92     |
| c2(95%)                  |     6.77     |     2.42     |     7.97     |     3.09     |
| Interval povjerenja(95%) | (3.92, 6.77) | (1.93, 2.42) | (1.48, 7.97) | (0.92, 3.09) |



![](https://github.com/NikolaJelic/ImageMerger/blob/main/resources/readme/512_graph.png)

### Ulaz veličine 256x256 bez kompajlerskih optimizacija

| Broj mjerenja            |    Bazni     |     Keš      |    OpenMP    | Optimizovani |
| ------------------------ | :----------: | :----------: | :----------: | :----------: |
| 1                        |     9.09     |     4.53     |     4.54     |     3.81     |
| 2                        |    17.67     |     3.15     |     6.39     |     2.40     |
| 3                        |     5.27     |     3.16     |     2.16     |     0.88     |
| 4                        |     4.09     |     3.66     |     3.42     |     1.01     |
| 5                        |     3.86     |     3.03     |     2.85     |     0.95     |
| 6                        |     3.77     |     3.14     |     2.13     |     0.93     |
| 7                        |     3.80     |     3.11     |     1.68     |     0.65     |
| 8                        |     3.75     |     3.16     |     1.64     |     1.10     |
| 9                        |     5.09     |     3.14     |     1.92     |     0.66     |
| 10                       |     3.95     |     3.04     |     1.90     |     0.68     |
| Aritmetička sredina      |     6.03     |     3.31     |     2.86     |     1.31     |
| Varijansa                |    19.38     |     0.21     |     2.36     |     1.03     |
| Standardna devijacija    |     4.40     |     0.46     |     1.54     |     1.02     |
| Koeficijent devijacije   |     0.73     |     0.14     |     0.54     |     0.78     |
| c1(90%)                  |     3.48     |     3.04     |     1.97     |     0.72     |
| c2(90%)                  |     8.59     |     3.58     |     3.75     |     1.89     |
| Interval povjerenja(90%) | (3.48, 8.59) | (3.04, 3.58) | (1.97, 3.75) | (0.72, 1.89) |
| c1(95%)                  |     2.88     |     2.98     |     1.76     |     0.58     |
| c2(95%)                  |     9.18     |     3.64     |     3.96     |     2.03     |
| Interval povjerenja(95%) | (2.88, 9.18) | (2.98, 3.64) | (1.76, 3.96) | (0.58, 2.03) |



### Ulaz veličine 256x256 sa kompajlerskim optimizacijama

| Broj mjerenja            |    Bazni     |     Keš      |     OpenMP     | Optimizovani  |
| ------------------------ | :----------: | :----------: | :------------: | :-----------: |
| 1                        |     1.55     |     0.49     |      0.63      |     5.52      |
| 2                        |     7.65     |     0.51     |      0.61      |     0.37      |
| 3                        |     7.22     |     4.12     |     30.66      |     6.24      |
| 4                        |     1.06     |     0.52     |      0.61      |     0.39      |
| 5                        |     1.03     |     0.56     |      0.66      |     0.37      |
| 6                        |     1.01     |     0.51     |      0.64      |     0.36      |
| 7                        |     1.28     |     0.47     |      0.65      |     0.39      |
| 8                        |     1.21     |     0.47     |      0.85      |     0.43      |
| 9                        |     1.36     |     0.57     |      1.91      |     0.39      |
| 10                       |     1.03     |     0.56     |      0.67      |     0.37      |
| Aritmetička sredina      |     2.44     |     0.88     |      3.79      |     1.48      |
| Varijansa                |     6.97     |     1.30     |     89.30      |     5.40      |
| Standardna devijacija    |     2.64     |     1.14     |      9.45      |     2.32      |
| Koeficijent devijacije   |     1.08     |     1.30     |      2.49      |     1.57      |
| c1(90%)                  |     0.91     |     0.22     |     -1.69      |     0.14      |
| c2(90%)                  |     3.97     |     1.54     |      9.27      |     2.83      |
| Interval povjerenja(90%) | (0.91, 3.97) | (0.22, 1.54) | (-1.69, 9.27)  | (0.14, 2.83)  |
| c1(95%)                  |     0.55     |     0.06     |     -2.97      |     -0.18     |
| c2(95%)                  |     4.33     |     1.69     |     10.55      |     3.15      |
| Interval povjerenja(95%) | (0.55, 4.33) | (0.06, 1.69) | (-2.97, 10.55) | (-0.18, 3.15) |



![](https://github.com/NikolaJelic/ImageMerger/blob/main/resources/readme/256_graph.png)

## Zaključak

Kompajlerske optimizacije konzistentno ubrzavaju sve implementacije algoritma za svaku ulaznu veličinu.
Pri različitim ulazima primjećuju se razlike u brzini izvršavanja između `keš` i `OpenMP` optimizacija. 

OpenMP optimizacije su dosta efikasnije pri većim ulazima, dok kod slika malih dimenzija, npr. 256x256 stvaranje tredova i okruženja koje openMP zapravo produžava vrijeme izvršavanja i tu pravilna upotreba keša zapravo donosi najbolje rezultate.

Kombinacija `OpenMP` i `keš` optimizacija generalno ima najbolje rezultate, ali se pri malim slikama i ovdje vidi nepotrebno usporavanje od strane `OpenMP`.

Kompajlerske optimizacije dodavanjem `-ofast`  značajno utiču na implementaciju optimizacije keša jer `-ofast` uključuje optmizacije koje se bave optmizacijom petlji, kao što su `-floop-unroll-and-jam` što znači da kompajler odmotava tijelo petlje, a zatim kombinuje rezultujuće odmotane petlje u jednu petlju ili `-ftree-loop-distribution` koja omogućava da se tijelo petlje razbije na manje dijelove, koji se onda mogu  izvršavati paralelno, što može smanjiti vrijeme izvršavanja. 

Još neke optimizacije koje se izvršavaju su: 

```
-fgcse-after-reload
-fipa-cp-clone
-floop-interchange
-floop-unroll-and-jam
-fpeel-loops
-fpredictive-commoning
-fsplit-loops
-fsplit-paths
-ftree-loop-distribution
-ftree-partial-pre
-funswitch-loops
-fvect-cost-model=dynamic
-fversion-loops-for-strides
```

Od kojih se veliki broj odnosi na optimizaciju iteracije kroz petlje.

Sve ove optimizacije doprinose povećanju rezultujućeg izvršnog fajla, što se vidi po tome da kompajler sa `o0` optimizacijama generiše fajl veličine 726.9kB , dok sa uključenim `ofast` optimizacijama generiše fajl veličine 1MB što je značajan rast.
