/*************************************************************
* proto_tdd_v3 -  récepteur                                  *
* TRANSFERT DE DONNEES  v3                                   *
*                                                            *
* Protocole Go-Back-N                                        *
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
    int window = 1;
    int borne_inf = 0;

    unsigned char message[MAX_INFO];
    paquet_t paquet, pack;
    pack.lg_info = 0;
    pack.type = NACK;
    int fin = 0;

    init_reseau(RECEPTION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    while ( !fin ) {
        de_reseau(&paquet);
        if (verifier_controle(&paquet) && dans_fenetre(borne_inf, paquet.num_seq, window)){
            pack.num_seq = paquet.num_seq;
            pack.type = ACK;
            pack.somme_ctrl = generer_controle(&pack);
            vers_reseau(&pack);
            borne_inf = inc(borne_inf, SEQ_NUM_SIZE);
            printf("[GAB] J'acquittes le paquet %d\n", pack.num_seq);
            for (int i=0; i<paquet.lg_info; i++) {
                message[i] = paquet.info[i];
            }
            printf("[GAB] Je transmets le paquet %d à l'application\n", pack.num_seq);
            fin = vers_application(message, paquet.lg_info);
        }
        else {
            if (!dans_fenetre(borne_inf, paquet.num_seq, window)){ // Hors fenetre
                printf("[TRP] Hors fenetre.\n");
                printf("[GAB] J'ai reçu %d mais j'attendais %d\n", paquet.num_seq, borne_inf);
            }
            else { // Erreur de somme de controle 
                printf("[TRP] Erreur de somme de controle.\n");
                printf("[GAB] J'ai reçu %d mais j'attendais %d du paquet %d\n", paquet.somme_ctrl, generer_controle(&paquet), paquet.num_seq);
            }
            printf("[GAB] Pack num : %d de Type : %d\n", pack.num_seq, pack.type);
            if (pack.type != NACK){ //envoie du dernier bon ack si il y en a un bien reçu sinon attend l'expiration du temporisateur
                vers_reseau(&pack); 
                printf("[GAB] J'envoie le dernier bon ack %d mais j'ai reçu %d\n", pack.num_seq, paquet.num_seq);
            }
        }
        
    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}