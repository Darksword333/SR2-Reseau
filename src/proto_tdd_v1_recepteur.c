/*************************************************************
* proto_tdd_v1 -  récepteur                                  *
* TRANSFERT DE DONNEES  v1                                   *
*                                                            *
* Protocole STOP-and-Wait Acquittement Négatif               *
*                                                            *
* MAZET Gabriel                                              *
**************************************************************/

#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

/* =============================== */
/* Programme principal - récepteur */
/* =============================== */
int main(int argc, char* argv[]){
    unsigned char message[MAX_INFO];
    paquet_t paquet, pack;
    pack.lg_info = 0;
    int fin = 0;

    init_reseau(RECEPTION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    while ( !fin ) {

        de_reseau(&paquet);
        if (verifier_controle(&paquet)) {
            pack.type = ACK;
            vers_reseau(&pack); // Permet d'envoyer plus vite l'ack pour recevoir le prochain paquet
            for (int i=0; i<paquet.lg_info; i++) {
                message[i] = paquet.info[i];
            }
            fin = vers_application(message, paquet.lg_info);
        }
        else {
            printf("[TRP] Erreur de somme de controle.\n");
            pack.type = NACK;
            vers_reseau(&pack);
        }
    }
    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}
