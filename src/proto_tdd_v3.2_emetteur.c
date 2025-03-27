/*************************************************************
* proto_tdd_v3.2 -  émetteur                                 *
* TRANSFERT DE DONNEES  v3.2                                 *
*                                                            *
* Protocole Go-Back-N     Fast Retransmit                    *
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

    while (taille_msg != 0) { //|| borne_inf-1 != curseur ) {
        if (dans_fenetre(borne_inf, curseur, window)){
            for (int i=0; i<taille_msg; i++) 
                paquet.info[i] = message[i];
            
            paquet.num_seq = curseur;
            paquet.lg_info = taille_msg;
            paquet.type = DATA;
            paquet.somme_ctrl = generer_controle(&paquet);
            buffer[curseur] = paquet;

            vers_reseau(&paquet);
            printf("[GAB] J'envoie le paquet %d-->\n", curseur);
            if (borne_inf == curseur) // Lancement du temporisateur si c'est le premier paquet de la fenêtre
                depart_temporisateur(100);
            curseur = inc(curseur, SEQ_NUM_SIZE);
            de_application(message, &taille_msg);
        }
        else {
            while(borne_inf != curseur){ 
                if ((evt = attendre()) == PAQUET_RECU){
                    de_reseau(&pack);
                    printf("[GAB] J'ai reçu le paquet %d<--\n", pack.num_seq);
                    if (verifier_controle(&pack) && dans_fenetre(borne_inf, pack.num_seq, window)){
                        borne_inf = inc(pack.num_seq, SEQ_NUM_SIZE);
                        printf("[GAB] Curseur : %d, Borne_inf : %d\n", borne_inf, curseur);
                    }
                    if (borne_inf == curseur) // Arrêt du temporisateur si c'est le dernier paquet de la fenêtre
                        arret_temporisateur();
                }
                else {
                    retransmit(borne_inf, curseur, buffer);
                }
            }
        }
    }
    i = 0; // Essaye Max_try fois de recevoir le dernier paquet sinon arrête
    curseur --; // Curseur est incrémenté une fois de trop
    while (pack.num_seq != curseur && max_try != i) { // Envoi du dernier paquet et vérification de la réception
        evt = attendre();
        if (evt == PAQUET_RECU){
            de_reseau(&pack);
            printf("[GAB] J'ai reçu le dernier paquet %d<--\n", pack.num_seq);
            if (verifier_controle(&pack) && curseur == pack.num_seq)
                arret_temporisateur();
            else {
                vers_reseau(&buffer[curseur]);
                depart_temporisateur(100);
                while(((evt = attendre()) != PAQUET_RECU) && max_try != i){ // Cas ou Reception d'ack mais mauvais ack
                    depart_temporisateur(100);
                    vers_reseau(&buffer[curseur]);
                    printf("[GAB] Je retransmets le dernier paquet %d-->\n", curseur);
                    i++;
                }
            }
        }
        else {
            vers_reseau(&buffer[curseur]);
            depart_temporisateur(100);
            while(((evt = attendre()) != PAQUET_RECU) && max_try != i){ // Cas ou Non Reception d'ack
                depart_temporisateur(100);
                vers_reseau(&buffer[curseur]);
                printf("[GAB] Je retransmets le dernier paquet %d-->\n", curseur);
                i++;
                }
            }
    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}