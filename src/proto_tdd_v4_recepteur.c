/*************************************************************
* proto_tdd_v4 -  récepteur                                  *
* TRANSFERT DE DONNEES  v4                                   *
*                                                            *
* Protocole Selective Repeat    ARQ                          *
*                                                            *
* MAZET Gabriel                                              *
**************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

/* =============================== */
/* Programme principal - récepteur */
/* =============================== */
int main(int argc, char* argv[]){
    int window = 4; // Prends la valeur 4 par défaut
    if (argc > 2) {
        printf("Usage: %s <window>\n", argv[0]);
        return 1;
    }
    if (argc == 2) { // Si un argument est passé, on le prend comme taille de la fenêtre
        window = atoi(argv[1]);
        if (window < 1 && window < ((SEQ_NUM_SIZE-1)/2)) {
            printf("Erreur Taille Fenetre\n");
            return 1;
        }
    }
    int borne_inf = 0, curseur = 0;
    unsigned char message[MAX_INFO];
    paquet_t paquet, pack;
    paquet_t buffer[SEQ_NUM_SIZE];
    int recu[SEQ_NUM_SIZE] = {0};
    pack.lg_info = 0;
    pack.type = ACK;
    int fin = 0;

    init_reseau(RECEPTION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    while ( !fin ) {
        de_reseau(&paquet);
        if (verifier_controle(&paquet) && dans_fenetre(borne_inf, paquet.num_seq, window)){
            pack.num_seq = paquet.num_seq;
            pack.somme_ctrl = generer_controle(&pack);
            vers_reseau(&pack);
            printf("[GAB] J'acquittes le paquet %d\n", pack.num_seq);
            if (paquet.num_seq != borne_inf){
                buffer[paquet.num_seq] = paquet;
                recu[paquet.num_seq] = 1;
            }
            else {
                borne_inf = inc(borne_inf, SEQ_NUM_SIZE);
                curseur = inc(curseur, SEQ_NUM_SIZE);
            }
            fin = check_and_deliver(buffer, recu, &borne_inf);
        }
        else {
            if (!verifier_controle(&paquet)){ // Erreur de somme de controle 
                printf("[TRP] Erreur de somme de controle.\n");
                printf("[GAB] J'ai reçu %d mais j'attendais %d du paquet %d\n", paquet.somme_ctrl, generer_controle(&paquet), paquet.num_seq);
            }
            else { // Hors fenetre
                printf("[TRP] Hors fenetre.\n");
                printf("[GAB] J'ai reçu %d mais j'attendais %d\n", paquet.num_seq, borne_inf);
                if (paquet.num_seq < borne_inf){
                    pack.num_seq = paquet.num_seq;
                    pack.type = ACK;
                    pack.somme_ctrl = generer_controle(&pack);
                    vers_reseau(&pack);
                    printf("[GAB] J'acquittes le paquet %d\n", pack.num_seq);
                }
                // Si c'est supérieur a la fenetre je ne fais rien 
                // j'attends de recevoir les paquets en fenetre via expiration de temporisateur
            }
        }
    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}