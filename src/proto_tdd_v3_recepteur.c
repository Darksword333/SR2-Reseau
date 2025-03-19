/*************************************************************
* proto_tdd_v3 -  récepteur                                  *
* TRANSFERT DE DONNEES  v3                                   *
*                                                            *
* Protocole Go-Back-N ARQ                                    *
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
int main(int argc, char* argv[])
{
    int window = 1;
    int curseur= 0;
    int borne_inf = 0;
    paquet_t fenetre[window];

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
            pack.type = ACK;
            pack.num_seq = paquet.num_seq;
            pack.somme_ctrl = generer_controle(&pack);
            vers_reseau(&pack);
            fenetre[paquet.num_seq] = paquet;
            while (curseur == borne_inf){ /* tant que j'ai des paquets en avance dans mon buffer fenetre*/
                borne_inf = inc(SEQ_NUM_SIZE, borne_inf);
                for (int i=0; i<fenetre[curseur].lg_info; i++) {
                    message[i] = fenetre[curseur].info[i];
                }
                fin = vers_application(message, paquet.lg_info);
            }
        }
        else {
            printf("[TRP] Erreur de somme de controle ou hors séquence.\n");
            if (pack.type == ACK)
                vers_reseau(&pack); //envoie du dernier bon ack si il y en a un bien reçu sinon attend l'expiration du temporisateur
        }
        
    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}