/*************************************************************
* proto_tdd_v2 -  récepteur                                  *
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
            vers_reseau(&pack);
            for (int i=0; i<paquet.lg_info; i++) {
                message[i] = paquet.info[i];
            }
            fin = vers_application(message, paquet.lg_info);
        }
        else {
            printf("[TRP] Erreur de somme de controle.\n");
            // N'envoie rien et attends pour faire expirer le temporisateur de l'emetteur
        }
        
    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}
