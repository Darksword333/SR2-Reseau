/*************************************************************
* proto_tdd_v2 -  émetteur                                   *
* TRANSFERT DE DONNEES  v2                                   *
*                                                            *
* Protocole STOP-and-Wait ARQ                                *
*                                                            *
* MAZET Gabriel                                              *
**************************************************************/

#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO];
    int taille_msg;
    paquet_t paquet;
    int evt;

    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    de_application(message, &taille_msg);

    while ( taille_msg != 0 ) {

        for (int i=0; i<taille_msg; i++) {
            paquet.info[i] = message[i];
        }
        paquet.lg_info = taille_msg;
        paquet.type = DATA;
        paquet.somme_ctrl = generer_controle(&paquet);

        vers_reseau(&paquet);
        depart_temporisateur(100);
        evt = attendre();
        while (evt != -1) {
            printf("[TRP] RéEmission paquet %d\n", paquet.num_seq);
            vers_reseau(&paquet);
            depart_temporisateur(100);
            evt = attendre();
        }
        // pas de problème de somme de controle donc on ne vérifie pas le paquet recu c'est forcement un ack qui ne contient pas d'erreur
        arret_temporisateur();
        de_application(message, &taille_msg);
    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}
