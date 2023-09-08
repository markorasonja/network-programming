#include <stdio.h>


// Ispisuje naredbe koje se mogu koristiti u programu
void PrintHelp(void){
    printf("pt... bot klijentima šalje poruku PROG_TCP (struct MSG:1 10.0.0.20 1234)\n");
    printf("ptl.. bot klijentima šalje poruku PROG_TCP (struct MSG:1 127.0.0.1 1234)\n");
    printf("pu... bot klijentima šalje poruku PROG_UDP (struct MSG:2 10.0.0.20 1234)\n");
    printf("pul.. bot klijentima šalje poruku PROG_UDP (struct MSG:2 127.0.0.1 1234)\n");
    printf("r.... bot klijentima šalje poruku RUN s adresama lokalnog računala:\n");
    printf("      struct MSG:3 127.0.0.1 vat localhost 6789\n");
    printf("r2... bot klijentima šalje poruku RUN s adresama računala iz IMUNES-a:\n");
    printf("      struct MSG:3 20.0.0.11 1111 20.0.0.12 2222 20.0.0.13 hostmon\n");
    printf("s.... bot klijentima šalje poruku STOP (struct MSG:4)\n");
    printf("l.... lokalni ispis adresa bot klijenata\n");
    printf("n.... šalje poruku: NEPOZNATA\n");
    printf("q.... bot klijentima šalje poruku QUIT i završava s radom (struct MSG:0)\n");
    printf("h.... ispis naredbi\n");
}
