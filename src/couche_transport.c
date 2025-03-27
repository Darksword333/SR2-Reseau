#include <stdio.h>
#include "couche_transport.h"
#include "services_reseau.h"
#include "application.h"

/* ************************************************************************** */
/* *************** Fonctions utilitaires couche transport ******************* */
/* ************************************************************************** */

int generer_controle(const paquet_t *paquet) {
    int somme = paquet->type ^ paquet->num_seq ^ paquet->lg_info;
    
    for (int i = 0; i < paquet->lg_info; i++) {
        somme ^= paquet->info[i];
    }
    
    return somme;
}

int verifier_controle(const paquet_t *paquet) {
    return generer_controle(paquet) == paquet->somme_ctrl;
}

int inc(int const n, int const mod){
    return (n+1) % mod;
}

void retransmit(int const borne_inf, int const curseur, paquet_t const *buffer) {
    int i = borne_inf;
    while (i != curseur) {
        printf("[GAB] Je retransmets le paquet %d-->\n", i);
        vers_reseau(&buffer[i]);
        i = inc(i, SEQ_NUM_SIZE);
    }
    depart_temporisateur(100);
}

int check_and_deliver(paquet_t buffer[], int recu[], int *borne_inf){
    int fin = 0;
    unsigned char message[MAX_INFO];
    while (recu[*borne_inf]) {
        for (int i=0; i<buffer[*borne_inf].lg_info; i++) {
            message[i] = buffer[*borne_inf].info[i];
        }
        fin = vers_application(message, buffer[*borne_inf].lg_info);
        recu[*borne_inf] = 0;  // Marquer comme traité
        *borne_inf = inc(borne_inf, SEQ_NUM_SIZE);
    }
    return fin;
}

/*--------------------------------------*/
/* Fonction d'inclusion dans la fenetre */
/*--------------------------------------*/
int dans_fenetre(unsigned int inf, unsigned int pointeur, int taille) {

    unsigned int sup = (inf+taille-1) % SEQ_NUM_SIZE;

    return
        /* inf <= pointeur <= sup */
        ( inf <= sup && pointeur >= inf && pointeur <= sup ) ||
        /* sup < inf <= pointeur */
        ( sup < inf && pointeur >= inf) ||
        /* pointeur <= sup < inf */
        ( sup < inf && pointeur <= sup);
}
