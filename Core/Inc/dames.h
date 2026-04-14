#ifndef DAMES_H
#define DAMES_H

#include "stdint.h"

#define NB_ETAPES_MAX_COUP      21U
#define TAILLE_MESSAGE_COUP_MAX 128U

typedef struct
{
  uint8_t ligne;
  uint8_t colonne;
} PositionCase;

typedef struct
{
  uint16_t numeroCoup;
  uint8_t nbEtapes;
  PositionCase etapes[NB_ETAPES_MAX_COUP];
} CoupDames;

typedef enum
{
  DAMES_MODE_LOCAL = 0,
  DAMES_MODE_UART
} DamesModePartie;

typedef enum
{
  DAMES_JOUEUR_LOCAL_BLANC = 0,
  DAMES_JOUEUR_LOCAL_NOIR
} DamesJoueurLocal;

typedef enum
{
  DAMES_ACTION_AUCUNE = 0,
  DAMES_ACTION_QUITTER
} DamesAction;

void Dames_ReinitialiserCoup(CoupDames *coup);
void Dames_InitialiserCoup(CoupDames *coup, uint16_t numeroCoup, PositionCase depart);
uint8_t Dames_AjouterEtapeCoup(CoupDames *coup, PositionCase etape);
uint8_t Dames_ConvertirCoupEnTexte(const CoupDames *coup, char *message, uint16_t tailleMessage);
uint8_t Dames_ConvertirTexteEnCoup(const char *message, CoupDames *coup);
uint8_t Dames_CoupLocalEstPret(void);
uint8_t Dames_RecupererDernierCoupLocal(CoupDames *coup);
void Dames_AcquitterDernierCoupLocal(void);
uint8_t Dames_AppliquerCoupRecu(const CoupDames *coup);

void Dames_AfficherNouvellePartie(DamesModePartie modePartie, DamesJoueurLocal joueurLocal);
DamesAction Dames_GererTouch(uint16_t x, uint16_t y);

#endif /* DAMES_H */
