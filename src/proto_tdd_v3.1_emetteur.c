/*************************************************************
* proto_tdd_v3.1 -  émetteur                                 *
* TRANSFERT DE DONNEES  v3.1                                 *
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
        printf("Usage: %s <window>\n", argv[0]);
        return 1;
    }
    if (argc == 2) {
        window = atoi(argv[1]); 
        if (window >= SEQ_NUM_SIZE) {
            printf("La taille de la fenêtre doit être inférieure à %d\n", SEQ_NUM_SIZE);
            return 1;
        }
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
                
            paquet.num_seq = curseur;
            paquet.lg_info = taille_msg;
            paquet.type = DATA;
            paquet.somme_ctrl = generer_controle(&paquet);
            fenetre[curseur] = paquet;

            vers_reseau(&paquet);
            printf("[GAB] J'envoie le paquet %d\n", curseur);
            if (borne_inf == curseur) // Lancement du temporisateur si c'est le premier paquet de la fenêtre
                depart_temporisateur(100);
            curseur ++;
        }
        else {
            evt = attendre();
            while(borne_inf != curseur){ // borne_inf-1 car borne_inf peut être égal à curseur
                if (evt == -1){ //Paquet Reçu
                    de_reseau(&pack);
                    printf("[GAB] J'ai reçu le paquet %d\n", pack.num_seq);
                    if (verifier_controle(&pack) && dans_fenetre(borne_inf, pack.num_seq, window))
                        borne_inf = pack.num_seq + 1;
                    arret_temporisateur();
                }
                //Sinon Temporisateur Expiré donc retransmission sans incrémentation de la borne_inf
                i = borne_inf;
                while (i != curseur) { // Retransmission de tous les paquets de la fenêtre
                    // peut etre construire les nouveaux paquets ici et les transmettres en decalant la fenetre
                    printf("[GAB] Je retransmets le paquet %d\n", i);
                    vers_reseau(&fenetre[i]);
                    i ++;
                }
                depart_temporisateur(100);
                evt = attendre();
            }
        }
        de_application(message, &taille_msg);
    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}
