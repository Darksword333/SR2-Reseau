/*************************************************************
* proto_tdd_v3 -  émetteur                                   *
* TRANSFERT DE DONNEES  v3                                   *
*                                                            *
* Protocole Go-Back-N     ARQ                                *
*                                                            *
* MAZET Gabriel                                              *
**************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */
int main(int argc, char* argv[]){
    int window = 4;
    if (argc > 2) {
        printf("Usage: %s <emetteur>\n", argv[0]);
        return 1;
    }
    if (argc == 2) {
        int val = atoi(argv[1]); 
        if (val >= SEQ_NUM_SIZE) {
            printf("La taille de la fenêtre doit être inférieure à %d\n", SEQ_NUM_SIZE);
            return 1;
        }
        window = val;
    }

    int curseur = 0, borne_inf = 0;
    paquet_t fenetre[window];
    int i;

    unsigned char message[MAX_INFO];
    int taille_msg;
    paquet_t paquet, pack;
    int evt;

    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    de_application(message, &taille_msg);

    while ( taille_msg != 0 ) {
        if (dans_fenetre(borne_inf, curseur, window)){
            for (int i=0; i<taille_msg; i++) 
                paquet.info[i] = message[i];
                
            paquet.lg_info = taille_msg;
            paquet.type = DATA;
            paquet.somme_ctrl = generer_controle(&paquet);
            fenetre[curseur] = paquet;

            vers_reseau(&paquet);
            if (borne_inf == curseur)
                depart_temporisateur(100);
            inc(SEQ_NUM_SIZE, &curseur);
        }
        else {
            evt = attendre();
            if (evt == -1) { //Paquet Recu
                de_reseau(&pack);
                if (verifier_controle(&pack) && dans_fenetre(borne_inf, pack.num_seq, window)) {
                    if (pack.num_seq == borne_inf) {
                        for (int i=0; i<pack.lg_info; i++) 
                            message[i] = pack.info[i];
                        taille_msg = pack.lg_info;
                        inc(SEQ_NUM_SIZE, &borne_inf);
                        if (borne_inf == curseur)
                            arret_temporisateur();
                    }
                    else {
                        printf("[TRP] Paquet ACK recu hors sequence.\n");
                    }
                }
            }
            else { //Temporisateur Expiré
                i = borne_inf;
                depart_temporisateur(100);
                while (i != curseur) {
                    vers_reseau(&fenetre[i]);
                    inc(SEQ_NUM_SIZE, &i);
                }
            }
        }
        de_application(message, &taille_msg);
    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}
