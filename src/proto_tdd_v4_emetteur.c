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
                        arret_temporisateur_num(pack.num_seq);
                    }
                }
                else {
                    vers_reseau(&buffer[evt]);
                    depart_temporisateur_num(evt, 100);
                }
            }
        }
    }
    // Essaye Max_try fois de recevoir le dernier paquet sinon arrête
    curseur --; // Curseur est incrémenté une fois de trop
    while (pack.num_seq != curseur && max_try != i) { // Envoi du dernier paquet et vérification de la réception
        evt = attendre();
        if (evt == PAQUET_RECU){
            de_reseau(&pack);
            if (verifier_controle(&pack) && curseur == pack.num_seq){
                arret_temporisateur_num(curseur);
                recu[curseur] = 1;
            }
            else {
                vers_reseau(&buffer[curseur]);
                depart_temporisateur_num(curseur, 100);
                while(((evt = attendre()) != PAQUET_RECU) && max_try != i){ // Cas ou Reception d'ack mais mauvais ack
                    depart_temporisateur_num(curseur, 100);
                    vers_reseau(&buffer[curseur]);
                    i++;
                }
            }
        }
        else {
            vers_reseau(&buffer[curseur]);
            depart_temporisateur_num(curseur, 100);
            while(((evt = attendre()) != PAQUET_RECU) && max_try != i){ // Cas ou Non Reception d'ack
                depart_temporisateur_num(curseur, 100);
                vers_reseau(&buffer[curseur]);
                i++;
                }
            }
    }
    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}