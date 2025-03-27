/*************************************************************
* proto_tdd_v4   -  émetteur                                 *
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
/* Programme principal - émetteur  */
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
    int curseur = 0, borne_inf = 0;
    paquet_t buffer[SEQ_NUM_SIZE];
    int recu[SEQ_NUM_SIZE] = {0};
    int i;

    unsigned char message[MAX_INFO];
    int taille_msg;
    paquet_t paquet, pack;
    int evt;

    int max_try = 10;

    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    de_application(message, &taille_msg);

    while (taille_msg != 0) { 
        if (dans_fenetre(borne_inf, curseur, window)){
            for (int i=0; i<taille_msg; i++) 
                paquet.info[i] = message[i];
            
            paquet.num_seq = curseur;
            paquet.lg_info = taille_msg;
            paquet.type = DATA;
            paquet.somme_ctrl = generer_controle(&paquet);
            buffer[curseur] = paquet;

            vers_reseau(&paquet);
            printf("[GAB] J'envoie le paquet %d\n", curseur);
            depart_temporisateur_num(curseur, 100);
            curseur = inc(curseur, SEQ_NUM_SIZE);
            de_application(message, &taille_msg);
        }
        else {
            while(borne_inf != curseur){ 
                if ((evt = attendre()) == PAQUET_RECU){
                    de_reseau(&pack);
                    if (verifier_controle(&pack) && dans_fenetre(borne_inf, pack.num_seq, window)){
                        recu[pack.num_seq] = 1;
                        if (borne_inf == pack.num_seq) {
                            while (recu[borne_inf] == 1) {
                                recu[borne_inf] = 0;
                                borne_inf = inc(borne_inf, SEQ_NUM_SIZE);
                            }
                        }
                        borne_inf = inc(pack.num_seq, SEQ_NUM_SIZE);
                        arret_temporisateur_num(pack.num_seq);
                        printf("[GAB] J'ai reçu l'acquittement du paquet %d\n", pack.num_seq);
                    }
                }
                else {
                    vers_reseau(&buffer[evt]);
                    printf("[GAB] Je retransmets le paquet %d\n", evt);
                    depart_temporisateur_num(evt, 100);
                }
            }
        }
    }
    return 0;
}