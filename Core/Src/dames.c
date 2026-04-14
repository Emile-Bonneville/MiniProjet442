#include "dames.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stm32746g_discovery_lcd.h"

#define TAILLE_PLATEAU             10U
#define TAILLE_CASE                24U
#define PLATEAU_X                  16U
#define PLATEAU_Y                  16U
#define TAILLE_PIXEL_PLATEAU       (TAILLE_PLATEAU * TAILLE_CASE)
#define EPAISSEUR_BORDURE_PLATEAU  2U
#define NB_LIGNES_PIONS            4U
#define RAYON_PION                 9U
#define RAYON_SYMBOLE_DAME         4U

#define COULEUR_CASE_CLAIRE        ((uint32_t)0xFFF1E3C6)
#define COULEUR_CASE_FONCEE        ((uint32_t)0xFF8B5A2B)
#define COULEUR_FOND_ECRAN         LCD_COLOR_WHITE
#define COULEUR_PION_BLANC         ((uint32_t)0xFFF7F3EB)
#define COULEUR_PION_NOIR          ((uint32_t)0xFF303030)
#define COULEUR_CONTOUR_PION_BLANC LCD_COLOR_BLACK
#define COULEUR_CONTOUR_PION_NOIR  LCD_COLOR_WHITE
#define COULEUR_SELECTION_CASE     LCD_COLOR_YELLOW
#define COULEUR_INFOS_JEU          LCD_COLOR_DARKBLUE
#define COULEUR_BOUTON_QUITTER     LCD_COLOR_RED
#define COULEUR_TEXTE_QUITTER      LCD_COLOR_WHITE

#define BOUTON_QUITTER_X           360U
#define BOUTON_QUITTER_Y           224U
#define BOUTON_QUITTER_LARGEUR     100U
#define BOUTON_QUITTER_HAUTEUR     32U

typedef enum
{
  CASE_VIDE = 0,
  PION_BLANC,
  PION_NOIR,
  DAME_BLANCHE,
  DAME_NOIRE
} TypeCase;

typedef enum
{
  JOUEUR_BLANC = 0,
  JOUEUR_NOIR
} TypeJoueur;

typedef struct
{
  TypeCase plateau[TAILLE_PLATEAU][TAILLE_PLATEAU];
  uint8_t piecesCaptureesEnCours[TAILLE_PLATEAU][TAILLE_PLATEAU];
  DamesModePartie modePartie;
  TypeJoueur joueurCourant;
  TypeJoueur joueurLocal;
  TypeJoueur gagnant;
  uint8_t selectionActive;
  uint8_t priseMultipleActive;
  uint8_t partieTerminee;
  uint8_t coupLocalPret;
  uint16_t prochainNumeroCoup;
  PositionCase caseSelectionnee;
  CoupDames coupEnCours;
  CoupDames dernierCoupLocal;
} EtatPartie;

static EtatPartie etatPartie;

static void InitialiserEtatCoups(EtatPartie *etat);
static void DemarrerCoupLocalSiNecessaire(EtatPartie *etat, PositionCase depart);
static void AjouterEtapeCoupLocal(EtatPartie *etat, PositionCase arrivee);
static void FinaliserCoupLocal(EtatPartie *etat);
static uint8_t AppliquerDeplacementOuPrise(EtatPartie *etat, PositionCase depart, PositionCase arrivee, uint8_t *priseEffectuee);
static void InitialiserPlateau(EtatPartie *etat);
static void PlacerPionsInitiaux(EtatPartie *etat);
static void InitialiserPartie(EtatPartie *etat);
static void ReinitialiserCapturesEnCours(EtatPartie *etat);
static uint8_t CoordonneesSontDansPlateau(int32_t ligne, int32_t colonne);
static uint8_t CoordonneesSontDansZone(uint16_t x, uint16_t y, uint16_t zoneX, uint16_t zoneY, uint16_t largeur, uint16_t hauteur);
static uint8_t JoueurLocalPeutJouer(const EtatPartie *etat);
static uint8_t CaseEstJouable(uint8_t ligne, uint8_t colonne);
static uint8_t CaseContientPieceDuJoueur(const EtatPartie *etat, uint8_t ligne, uint8_t colonne);
static uint8_t CaseContientPieceAdverse(const EtatPartie *etat, uint8_t ligne, uint8_t colonne);
static uint8_t CaseEstVide(const EtatPartie *etat, uint8_t ligne, uint8_t colonne);
static uint8_t CaseEstDejaCaptureeEnCours(const EtatPartie *etat, uint8_t ligne, uint8_t colonne);
static uint8_t ConvertirCoordonneesEnCase(uint16_t xTactile, uint16_t yTactile, PositionCase *caseTouchee);
static uint8_t DeplacementSimplePionEstValide(const EtatPartie *etat, PositionCase depart, PositionCase arrivee);
static uint8_t DeplacementSimpleDameEstValide(const EtatPartie *etat, PositionCase depart, PositionCase arrivee);
static uint8_t DeplacementSimpleEstValide(const EtatPartie *etat, PositionCase depart, PositionCase arrivee);
static uint8_t PriseSimplePionEstValide(const EtatPartie *etat, PositionCase depart, PositionCase arrivee, PositionCase *caseCapturee);
static uint8_t PriseSimpleDameEstValide(const EtatPartie *etat, PositionCase depart, PositionCase arrivee, PositionCase *caseCapturee);
static uint8_t PriseSimpleEstValide(const EtatPartie *etat, PositionCase depart, PositionCase arrivee, PositionCase *caseCapturee);
static uint8_t CalculerMaxPrisesDepuis(const EtatPartie *etat, PositionCase depart);
static uint8_t CalculerMaxPrisesJoueur(const EtatPartie *etat);
static uint8_t PriseRespecteLeMaximum(const EtatPartie *etat, PositionCase depart, PositionCase arrivee, PositionCase *caseCapturee);
static uint8_t PiecePeutCapturerDepuis(const EtatPartie *etat, PositionCase depart);
static uint8_t JoueurDoitCapturer(const EtatPartie *etat);
static uint8_t PiecePeutEtreSelectionnee(const EtatPartie *etat, PositionCase caseTouchee);
static uint8_t PiecePeutSeDeplacerDepuis(const EtatPartie *etat, PositionCase depart);
static uint8_t JoueurPeutJouer(const EtatPartie *etat, TypeJoueur joueur);
static void EvaluerFinPartie(EtatPartie *etat);
static void ChangerJoueurCourant(EtatPartie *etat);
static void PromouvoirPionSiNecessaire(EtatPartie *etat, PositionCase arrivee);
static void DeplacerPiece(EtatPartie *etat, PositionCase depart, PositionCase arrivee);
static void EffectuerPriseSimple(EtatPartie *etat, PositionCase depart, PositionCase arrivee, PositionCase caseCapturee);
static void RetirerPiecesCaptureesEnCours(EtatPartie *etat);
static void FinaliserTourSansCapture(EtatPartie *etat, PositionCase arrivee);
static void FinaliserTourApresCapture(EtatPartie *etat, PositionCase arrivee);
static void DeselectionnerCase(EtatPartie *etat);
static void SelectionnerCase(EtatPartie *etat, uint8_t ligne, uint8_t colonne);
static void DessinerPlateau(void);
static void ObtenirCentreCase(uint32_t ligne, uint32_t colonne, uint16_t *x, uint16_t *y);
static void AfficherTexteCentreZone(uint16_t x, uint16_t y, uint16_t largeur, const char *texte);
static void DessinerPion(uint32_t ligne, uint32_t colonne, uint32_t couleurRemplissage, uint32_t couleurContour);
static void DessinerSymboleDame(uint32_t ligne, uint32_t colonne, uint32_t couleurSymbole);
static void DessinerPions(const EtatPartie *etat);
static void DessinerSelection(const EtatPartie *etat);
static void DessinerBoutonQuitter(void);
static void DessinerInfosJeu(const EtatPartie *etat);
static void DessinerElementsJeu(const EtatPartie *etat);

void Dames_ReinitialiserCoup(CoupDames *coup)
{
  memset(coup, 0, sizeof(*coup));
}

void Dames_InitialiserCoup(CoupDames *coup, uint16_t numeroCoup, PositionCase depart)
{
  Dames_ReinitialiserCoup(coup);
  coup->numeroCoup = numeroCoup;
  coup->nbEtapes = 1U;
  coup->etapes[0] = depart;
}

uint8_t Dames_AjouterEtapeCoup(CoupDames *coup, PositionCase etape)
{
  if (coup->nbEtapes >= NB_ETAPES_MAX_COUP)
  {
    return 0U;
  }

  coup->etapes[coup->nbEtapes] = etape;
  coup->nbEtapes++;
  return 1U;
}

uint8_t Dames_ConvertirCoupEnTexte(const CoupDames *coup, char *message, uint16_t tailleMessage)
{
  int longueurEcrite;
  uint16_t positionCourante;
  uint32_t indexEtape;

  if ((message == NULL) || (tailleMessage == 0U) || (coup->nbEtapes < 2U))
  {
    return 0U;
  }

  longueurEcrite = snprintf(message, tailleMessage, "COUP;%u;%u;",
                            (unsigned int)coup->numeroCoup,
                            (unsigned int)coup->nbEtapes);
  if ((longueurEcrite < 0) || ((uint16_t)longueurEcrite >= tailleMessage))
  {
    return 0U;
  }

  positionCourante = (uint16_t)longueurEcrite;

  for (indexEtape = 0; indexEtape < coup->nbEtapes; indexEtape++)
  {
    longueurEcrite = snprintf(&message[positionCourante], (uint16_t)(tailleMessage - positionCourante),
                              "%u,%u;",
                              (unsigned int)coup->etapes[indexEtape].ligne,
                              (unsigned int)coup->etapes[indexEtape].colonne);
    if ((longueurEcrite < 0) || ((uint16_t)longueurEcrite >= (uint16_t)(tailleMessage - positionCourante)))
    {
      return 0U;
    }

    positionCourante = (uint16_t)(positionCourante + (uint16_t)longueurEcrite);
  }

  if ((positionCourante + 1U) >= tailleMessage)
  {
    return 0U;
  }

  message[positionCourante] = '\n';
  message[positionCourante + 1U] = '\0';
  return 1U;
}

uint8_t Dames_ConvertirTexteEnCoup(const char *message, CoupDames *coup)
{
  char copieMessage[TAILLE_MESSAGE_COUP_MAX];
  char *jeton;
  char *contexte;
  char *separateurVirgule;
  uint32_t indexEtape;
  unsigned long numeroCoup;
  unsigned long nbEtapes;

  if ((message == NULL) || (coup == NULL) || (strlen(message) >= sizeof(copieMessage)))
  {
    return 0U;
  }

  strncpy(copieMessage, message, sizeof(copieMessage) - 1U);
  copieMessage[sizeof(copieMessage) - 1U] = '\0';

  jeton = strtok_r(copieMessage, ";\n\r", &contexte);
  if ((jeton == NULL) || (strcmp(jeton, "COUP") != 0))
  {
    return 0U;
  }

  jeton = strtok_r(NULL, ";\n\r", &contexte);
  if (jeton == NULL)
  {
    return 0U;
  }
  numeroCoup = strtoul(jeton, NULL, 10);

  jeton = strtok_r(NULL, ";\n\r", &contexte);
  if (jeton == NULL)
  {
    return 0U;
  }
  nbEtapes = strtoul(jeton, NULL, 10);

  if ((nbEtapes < 2UL) || (nbEtapes > NB_ETAPES_MAX_COUP))
  {
    return 0U;
  }

  Dames_ReinitialiserCoup(coup);
  coup->numeroCoup = (uint16_t)numeroCoup;
  coup->nbEtapes = (uint8_t)nbEtapes;

  for (indexEtape = 0; indexEtape < coup->nbEtapes; indexEtape++)
  {
    unsigned long ligne;
    unsigned long colonne;

    jeton = strtok_r(NULL, ";\n\r", &contexte);
    if (jeton == NULL)
    {
      return 0U;
    }

    separateurVirgule = strchr(jeton, ',');
    if (separateurVirgule == NULL)
    {
      return 0U;
    }

    *separateurVirgule = '\0';
    ligne = strtoul(jeton, NULL, 10);
    colonne = strtoul(separateurVirgule + 1, NULL, 10);

    if ((ligne >= TAILLE_PLATEAU) || (colonne >= TAILLE_PLATEAU))
    {
      return 0U;
    }

    coup->etapes[indexEtape].ligne = (uint8_t)ligne;
    coup->etapes[indexEtape].colonne = (uint8_t)colonne;
  }

  return 1U;
}

uint8_t Dames_CoupLocalEstPret(void)
{
  return etatPartie.coupLocalPret;
}

uint8_t Dames_RecupererDernierCoupLocal(CoupDames *coup)
{
  if ((coup == NULL) || (etatPartie.coupLocalPret == 0U))
  {
    return 0U;
  }

  *coup = etatPartie.dernierCoupLocal;
  return 1U;
}

void Dames_AcquitterDernierCoupLocal(void)
{
  etatPartie.coupLocalPret = 0U;
}

uint8_t Dames_AppliquerCoupRecu(const CoupDames *coup)
{
  uint8_t indexEtape;
  uint8_t priseEffectuee = 0U;

  if ((coup == NULL) ||
      (coup->nbEtapes < 2U) ||
      (coup->numeroCoup != etatPartie.prochainNumeroCoup) ||
      (etatPartie.partieTerminee != 0U))
  {
    return 0U;
  }

  DeselectionnerCase(&etatPartie);
  ReinitialiserCapturesEnCours(&etatPartie);
  Dames_ReinitialiserCoup(&etatPartie.coupEnCours);

  for (indexEtape = 0U; indexEtape < (uint8_t)(coup->nbEtapes - 1U); indexEtape++)
  {
    if (AppliquerDeplacementOuPrise(&etatPartie, coup->etapes[indexEtape], coup->etapes[indexEtape + 1U], &priseEffectuee) == 0U)
    {
      ReinitialiserCapturesEnCours(&etatPartie);
      return 0U;
    }
  }

  if (priseEffectuee != 0U)
  {
    FinaliserTourApresCapture(&etatPartie, coup->etapes[coup->nbEtapes - 1U]);
  }
  else
  {
    FinaliserTourSansCapture(&etatPartie, coup->etapes[coup->nbEtapes - 1U]);
  }

  etatPartie.prochainNumeroCoup = (uint16_t)(coup->numeroCoup + 1U);
  etatPartie.coupLocalPret = 0U;
  Dames_ReinitialiserCoup(&etatPartie.coupEnCours);
  DessinerElementsJeu(&etatPartie);
  return 1U;
}

void Dames_AfficherNouvellePartie(DamesModePartie modePartie, DamesJoueurLocal joueurLocal)
{
  InitialiserPartie(&etatPartie);
  etatPartie.modePartie = modePartie;
  etatPartie.joueurLocal = (joueurLocal == DAMES_JOUEUR_LOCAL_NOIR) ? JOUEUR_NOIR : JOUEUR_BLANC;
  DessinerPlateau();
  DessinerElementsJeu(&etatPartie);
}

DamesAction Dames_GererTouch(uint16_t x, uint16_t y)
{
  PositionCase caseTouchee = {0};
  PositionCase caseDepart = {0};
  PositionCase caseCapturee = {0};
  uint8_t priseObligatoire;

  if (CoordonneesSontDansZone(x, y, BOUTON_QUITTER_X, BOUTON_QUITTER_Y,
                              BOUTON_QUITTER_LARGEUR, BOUTON_QUITTER_HAUTEUR) != 0U)
  {
    return DAMES_ACTION_QUITTER;
  }

  if ((etatPartie.partieTerminee != 0U) ||
      (ConvertirCoordonneesEnCase(x, y, &caseTouchee) == 0U))
  {
    return DAMES_ACTION_AUCUNE;
  }

  if (JoueurLocalPeutJouer(&etatPartie) == 0U)
  {
    return DAMES_ACTION_AUCUNE;
  }

  priseObligatoire = JoueurDoitCapturer(&etatPartie);

  if (etatPartie.selectionActive == 0U)
  {
    if (PiecePeutEtreSelectionnee(&etatPartie, caseTouchee) != 0U)
    {
      SelectionnerCase(&etatPartie, caseTouchee.ligne, caseTouchee.colonne);
    }
  }
  else
  {
    caseDepart = etatPartie.caseSelectionnee;

    if (etatPartie.priseMultipleActive != 0U)
    {
      if (PriseRespecteLeMaximum(&etatPartie, caseDepart, caseTouchee, &caseCapturee) != 0U)
      {
        DemarrerCoupLocalSiNecessaire(&etatPartie, caseDepart);
        EffectuerPriseSimple(&etatPartie, caseDepart, caseTouchee, caseCapturee);
        AjouterEtapeCoupLocal(&etatPartie, caseTouchee);
        SelectionnerCase(&etatPartie, caseTouchee.ligne, caseTouchee.colonne);

        if (PiecePeutCapturerDepuis(&etatPartie, caseTouchee) != 0U)
        {
          etatPartie.priseMultipleActive = 1U;
        }
        else
        {
          FinaliserTourApresCapture(&etatPartie, caseTouchee);
        }
      }
    }
    else if ((caseDepart.ligne == caseTouchee.ligne) &&
             (caseDepart.colonne == caseTouchee.colonne))
    {
      DeselectionnerCase(&etatPartie);
    }
    else if (PiecePeutEtreSelectionnee(&etatPartie, caseTouchee) != 0U)
    {
      SelectionnerCase(&etatPartie, caseTouchee.ligne, caseTouchee.colonne);
    }
    else if (PriseRespecteLeMaximum(&etatPartie, caseDepart, caseTouchee, &caseCapturee) != 0U)
    {
      DemarrerCoupLocalSiNecessaire(&etatPartie, caseDepart);
      EffectuerPriseSimple(&etatPartie, caseDepart, caseTouchee, caseCapturee);
      AjouterEtapeCoupLocal(&etatPartie, caseTouchee);
      SelectionnerCase(&etatPartie, caseTouchee.ligne, caseTouchee.colonne);

      if (PiecePeutCapturerDepuis(&etatPartie, caseTouchee) != 0U)
      {
        etatPartie.priseMultipleActive = 1U;
      }
      else
      {
        FinaliserTourApresCapture(&etatPartie, caseTouchee);
      }
    }
    else if ((priseObligatoire == 0U) &&
             (DeplacementSimpleEstValide(&etatPartie, caseDepart, caseTouchee) != 0U))
    {
      DemarrerCoupLocalSiNecessaire(&etatPartie, caseDepart);
      DeplacerPiece(&etatPartie, caseDepart, caseTouchee);
      AjouterEtapeCoupLocal(&etatPartie, caseTouchee);
      FinaliserTourSansCapture(&etatPartie, caseTouchee);
    }
  }

  DessinerElementsJeu(&etatPartie);
  return DAMES_ACTION_AUCUNE;
}

static void InitialiserEtatCoups(EtatPartie *etat)
{
  etat->coupLocalPret = 0U;
  etat->prochainNumeroCoup = 1U;
  Dames_ReinitialiserCoup(&etat->coupEnCours);
  Dames_ReinitialiserCoup(&etat->dernierCoupLocal);
}

static void DemarrerCoupLocalSiNecessaire(EtatPartie *etat, PositionCase depart)
{
  if (etat->coupEnCours.nbEtapes != 0U)
  {
    return;
  }

  Dames_InitialiserCoup(&etat->coupEnCours, etat->prochainNumeroCoup, depart);
}

static void AjouterEtapeCoupLocal(EtatPartie *etat, PositionCase arrivee)
{
  if (etat->coupEnCours.nbEtapes == 0U)
  {
    return;
  }

  (void)Dames_AjouterEtapeCoup(&etat->coupEnCours, arrivee);
}

static void FinaliserCoupLocal(EtatPartie *etat)
{
  if (etat->coupEnCours.nbEtapes < 2U)
  {
    Dames_ReinitialiserCoup(&etat->coupEnCours);
    return;
  }

  etat->dernierCoupLocal = etat->coupEnCours;
  etat->coupLocalPret = 1U;
  etat->prochainNumeroCoup++;
  Dames_ReinitialiserCoup(&etat->coupEnCours);
}

static uint8_t AppliquerDeplacementOuPrise(EtatPartie *etat, PositionCase depart, PositionCase arrivee, uint8_t *priseEffectuee)
{
  PositionCase caseCapturee = {0};

  if (PriseSimpleEstValide(etat, depart, arrivee, &caseCapturee) != 0U)
  {
    EffectuerPriseSimple(etat, depart, arrivee, caseCapturee);
    *priseEffectuee = 1U;
    return 1U;
  }

  if ((*priseEffectuee == 0U) &&
      (DeplacementSimpleEstValide(etat, depart, arrivee) != 0U))
  {
    DeplacerPiece(etat, depart, arrivee);
    return 1U;
  }

  return 0U;
}

static void InitialiserPlateau(EtatPartie *etat)
{
  uint32_t ligne;
  uint32_t colonne;

  for (ligne = 0; ligne < TAILLE_PLATEAU; ligne++)
  {
    for (colonne = 0; colonne < TAILLE_PLATEAU; colonne++)
    {
      etat->plateau[ligne][colonne] = CASE_VIDE;
    }
  }
}

static void PlacerPionsInitiaux(EtatPartie *etat)
{
  uint32_t ligne;
  uint32_t colonne;

  for (ligne = 0; ligne < NB_LIGNES_PIONS; ligne++)
  {
    for (colonne = 0; colonne < TAILLE_PLATEAU; colonne++)
    {
      if (((ligne + colonne) % 2U) != 0U)
      {
        etat->plateau[ligne][colonne] = PION_BLANC;
      }
    }
  }

  for (ligne = TAILLE_PLATEAU - NB_LIGNES_PIONS; ligne < TAILLE_PLATEAU; ligne++)
  {
    for (colonne = 0; colonne < TAILLE_PLATEAU; colonne++)
    {
      if (((ligne + colonne) % 2U) != 0U)
      {
        etat->plateau[ligne][colonne] = PION_NOIR;
      }
    }
  }
}

static void InitialiserPartie(EtatPartie *etat)
{
  InitialiserPlateau(etat);
  PlacerPionsInitiaux(etat);
  ReinitialiserCapturesEnCours(etat);
  InitialiserEtatCoups(etat);

  etat->modePartie = DAMES_MODE_LOCAL;
  etat->joueurCourant = JOUEUR_BLANC;
  etat->joueurLocal = JOUEUR_BLANC;
  etat->gagnant = JOUEUR_BLANC;
  etat->selectionActive = 0U;
  etat->priseMultipleActive = 0U;
  etat->partieTerminee = 0U;
  etat->caseSelectionnee.ligne = 0U;
  etat->caseSelectionnee.colonne = 0U;
}

static void ReinitialiserCapturesEnCours(EtatPartie *etat)
{
  uint32_t ligne;
  uint32_t colonne;

  for (ligne = 0; ligne < TAILLE_PLATEAU; ligne++)
  {
    for (colonne = 0; colonne < TAILLE_PLATEAU; colonne++)
    {
      etat->piecesCaptureesEnCours[ligne][colonne] = 0U;
    }
  }
}

static uint8_t CoordonneesSontDansPlateau(int32_t ligne, int32_t colonne)
{
  return (uint8_t)((ligne >= 0) && (ligne < (int32_t)TAILLE_PLATEAU) &&
                   (colonne >= 0) && (colonne < (int32_t)TAILLE_PLATEAU));
}

static uint8_t CoordonneesSontDansZone(uint16_t x, uint16_t y, uint16_t zoneX, uint16_t zoneY, uint16_t largeur, uint16_t hauteur)
{
  return (uint8_t)((x >= zoneX) && (x < (zoneX + largeur)) &&
                   (y >= zoneY) && (y < (zoneY + hauteur)));
}

static uint8_t JoueurLocalPeutJouer(const EtatPartie *etat)
{
  if (etat->modePartie == DAMES_MODE_LOCAL)
  {
    return 1U;
  }

  return (uint8_t)(etat->joueurCourant == etat->joueurLocal);
}

static uint8_t CaseEstJouable(uint8_t ligne, uint8_t colonne)
{
  return (uint8_t)(((ligne + colonne) % 2U) != 0U);
}

static uint8_t CaseContientPieceDuJoueur(const EtatPartie *etat, uint8_t ligne, uint8_t colonne)
{
  TypeCase typeCase = etat->plateau[ligne][colonne];

  if (etat->joueurCourant == JOUEUR_BLANC)
  {
    return (uint8_t)((typeCase == PION_BLANC) || (typeCase == DAME_BLANCHE));
  }

  return (uint8_t)((typeCase == PION_NOIR) || (typeCase == DAME_NOIRE));
}

static uint8_t CaseContientPieceAdverse(const EtatPartie *etat, uint8_t ligne, uint8_t colonne)
{
  TypeCase typeCase = etat->plateau[ligne][colonne];

  if (CaseEstDejaCaptureeEnCours(etat, ligne, colonne) != 0U)
  {
    return 0U;
  }

  if (etat->joueurCourant == JOUEUR_BLANC)
  {
    return (uint8_t)((typeCase == PION_NOIR) || (typeCase == DAME_NOIRE));
  }

  return (uint8_t)((typeCase == PION_BLANC) || (typeCase == DAME_BLANCHE));
}

static uint8_t CaseEstVide(const EtatPartie *etat, uint8_t ligne, uint8_t colonne)
{
  return (uint8_t)(etat->plateau[ligne][colonne] == CASE_VIDE);
}

static uint8_t CaseEstDejaCaptureeEnCours(const EtatPartie *etat, uint8_t ligne, uint8_t colonne)
{
  return etat->piecesCaptureesEnCours[ligne][colonne];
}

static uint8_t ConvertirCoordonneesEnCase(uint16_t xTactile, uint16_t yTactile, PositionCase *caseTouchee)
{
  if ((xTactile < PLATEAU_X) || (xTactile >= (PLATEAU_X + TAILLE_PIXEL_PLATEAU)) ||
      (yTactile < PLATEAU_Y) || (yTactile >= (PLATEAU_Y + TAILLE_PIXEL_PLATEAU)))
  {
    return 0U;
  }

  caseTouchee->colonne = (uint8_t)((xTactile - PLATEAU_X) / TAILLE_CASE);
  caseTouchee->ligne = (uint8_t)((yTactile - PLATEAU_Y) / TAILLE_CASE);

  return 1U;
}

static uint8_t DeplacementSimplePionEstValide(const EtatPartie *etat, PositionCase depart, PositionCase arrivee)
{
  int32_t differenceLigne = (int32_t)arrivee.ligne - (int32_t)depart.ligne;
  int32_t differenceColonne = (int32_t)arrivee.colonne - (int32_t)depart.colonne;
  TypeCase typeCase = etat->plateau[depart.ligne][depart.colonne];

  if ((CaseEstJouable(arrivee.ligne, arrivee.colonne) == 0U) ||
      (CaseEstVide(etat, arrivee.ligne, arrivee.colonne) == 0U))
  {
    return 0U;
  }

  if ((differenceColonne != -1) && (differenceColonne != 1))
  {
    return 0U;
  }

  if ((typeCase == PION_BLANC) && (differenceLigne == 1))
  {
    return 1U;
  }

  if ((typeCase == PION_NOIR) && (differenceLigne == -1))
  {
    return 1U;
  }

  return 0U;
}

static uint8_t DeplacementSimpleDameEstValide(const EtatPartie *etat, PositionCase depart, PositionCase arrivee)
{
  int32_t differenceLigne = (int32_t)arrivee.ligne - (int32_t)depart.ligne;
  int32_t differenceColonne = (int32_t)arrivee.colonne - (int32_t)depart.colonne;
  int32_t pasLigne;
  int32_t pasColonne;
  int32_t ligneCourante;
  int32_t colonneCourante;

  if ((differenceLigne == 0) || (differenceColonne == 0) ||
      ((differenceLigne > 0 ? differenceLigne : -differenceLigne) !=
       (differenceColonne > 0 ? differenceColonne : -differenceColonne)))
  {
    return 0U;
  }

  if ((CaseEstJouable(arrivee.ligne, arrivee.colonne) == 0U) ||
      (CaseEstVide(etat, arrivee.ligne, arrivee.colonne) == 0U))
  {
    return 0U;
  }

  pasLigne = (differenceLigne > 0) ? 1 : -1;
  pasColonne = (differenceColonne > 0) ? 1 : -1;
  ligneCourante = (int32_t)depart.ligne + pasLigne;
  colonneCourante = (int32_t)depart.colonne + pasColonne;

  while ((ligneCourante != (int32_t)arrivee.ligne) && (colonneCourante != (int32_t)arrivee.colonne))
  {
    if (etat->plateau[ligneCourante][colonneCourante] != CASE_VIDE)
    {
      return 0U;
    }

    ligneCourante += pasLigne;
    colonneCourante += pasColonne;
  }

  return 1U;
}

static uint8_t DeplacementSimpleEstValide(const EtatPartie *etat, PositionCase depart, PositionCase arrivee)
{
  TypeCase typeCase = etat->plateau[depart.ligne][depart.colonne];

  if ((typeCase == PION_BLANC) || (typeCase == PION_NOIR))
  {
    return DeplacementSimplePionEstValide(etat, depart, arrivee);
  }

  if ((typeCase == DAME_BLANCHE) || (typeCase == DAME_NOIRE))
  {
    return DeplacementSimpleDameEstValide(etat, depart, arrivee);
  }

  return 0U;
}

static uint8_t PriseSimplePionEstValide(const EtatPartie *etat, PositionCase depart, PositionCase arrivee, PositionCase *caseCapturee)
{
  int32_t differenceLigne = (int32_t)arrivee.ligne - (int32_t)depart.ligne;
  int32_t differenceColonne = (int32_t)arrivee.colonne - (int32_t)depart.colonne;

  if ((CaseEstJouable(arrivee.ligne, arrivee.colonne) == 0U) ||
      (CaseEstVide(etat, arrivee.ligne, arrivee.colonne) == 0U))
  {
    return 0U;
  }

  if (!(((differenceLigne == 2) || (differenceLigne == -2)) &&
        ((differenceColonne == 2) || (differenceColonne == -2))))
  {
    return 0U;
  }

  caseCapturee->ligne = (uint8_t)((depart.ligne + arrivee.ligne) / 2U);
  caseCapturee->colonne = (uint8_t)((depart.colonne + arrivee.colonne) / 2U);

  return CaseContientPieceAdverse(etat, caseCapturee->ligne, caseCapturee->colonne);
}

static uint8_t PriseSimpleDameEstValide(const EtatPartie *etat, PositionCase depart, PositionCase arrivee, PositionCase *caseCapturee)
{
  int32_t differenceLigne = (int32_t)arrivee.ligne - (int32_t)depart.ligne;
  int32_t differenceColonne = (int32_t)arrivee.colonne - (int32_t)depart.colonne;
  int32_t pasLigne;
  int32_t pasColonne;
  int32_t ligneCourante;
  int32_t colonneCourante;
  uint8_t pieceAdverseTrouvee = 0U;

  if ((differenceLigne == 0) || (differenceColonne == 0) ||
      ((differenceLigne > 0 ? differenceLigne : -differenceLigne) !=
       (differenceColonne > 0 ? differenceColonne : -differenceColonne)))
  {
    return 0U;
  }

  if ((CaseEstJouable(arrivee.ligne, arrivee.colonne) == 0U) ||
      (CaseEstVide(etat, arrivee.ligne, arrivee.colonne) == 0U))
  {
    return 0U;
  }

  pasLigne = (differenceLigne > 0) ? 1 : -1;
  pasColonne = (differenceColonne > 0) ? 1 : -1;
  ligneCourante = (int32_t)depart.ligne + pasLigne;
  colonneCourante = (int32_t)depart.colonne + pasColonne;

  while ((ligneCourante != (int32_t)arrivee.ligne) && (colonneCourante != (int32_t)arrivee.colonne))
  {
    if (etat->plateau[ligneCourante][colonneCourante] != CASE_VIDE)
    {
      if (CaseEstDejaCaptureeEnCours(etat, (uint8_t)ligneCourante, (uint8_t)colonneCourante) != 0U)
      {
        return 0U;
      }

      if (CaseContientPieceAdverse(etat, (uint8_t)ligneCourante, (uint8_t)colonneCourante) == 0U)
      {
        return 0U;
      }

      if (pieceAdverseTrouvee != 0U)
      {
        return 0U;
      }

      pieceAdverseTrouvee = 1U;
      caseCapturee->ligne = (uint8_t)ligneCourante;
      caseCapturee->colonne = (uint8_t)colonneCourante;
    }

    ligneCourante += pasLigne;
    colonneCourante += pasColonne;
  }

  return pieceAdverseTrouvee;
}

static uint8_t PriseSimpleEstValide(const EtatPartie *etat, PositionCase depart, PositionCase arrivee, PositionCase *caseCapturee)
{
  TypeCase typeCase = etat->plateau[depart.ligne][depart.colonne];

  if ((typeCase == PION_BLANC) || (typeCase == PION_NOIR))
  {
    return PriseSimplePionEstValide(etat, depart, arrivee, caseCapturee);
  }

  if ((typeCase == DAME_BLANCHE) || (typeCase == DAME_NOIRE))
  {
    return PriseSimpleDameEstValide(etat, depart, arrivee, caseCapturee);
  }

  return 0U;
}

static uint8_t CalculerMaxPrisesDepuis(const EtatPartie *etat, PositionCase depart)
{
  TypeCase typeCase = etat->plateau[depart.ligne][depart.colonne];
  uint8_t maximum = 0U;

  if ((typeCase == PION_BLANC) || (typeCase == PION_NOIR))
  {
    int32_t directions[4][2] = {
      {-1, -1}, {-1, 1}, {1, -1}, {1, 1}
    };
    uint32_t indexDirection;

    for (indexDirection = 0; indexDirection < 4U; indexDirection++)
    {
      PositionCase caseArrivee;
      PositionCase caseCapturee;
      EtatPartie copieEtat;
      uint8_t nbPrises;

      if (CoordonneesSontDansPlateau((int32_t)depart.ligne + (2 * directions[indexDirection][0]),
                                     (int32_t)depart.colonne + (2 * directions[indexDirection][1])) == 0U)
      {
        continue;
      }

      caseArrivee.ligne = (uint8_t)((int32_t)depart.ligne + (2 * directions[indexDirection][0]));
      caseArrivee.colonne = (uint8_t)((int32_t)depart.colonne + (2 * directions[indexDirection][1]));

      if (PriseSimplePionEstValide(etat, depart, caseArrivee, &caseCapturee) != 0U)
      {
        copieEtat = *etat;
        EffectuerPriseSimple(&copieEtat, depart, caseArrivee, caseCapturee);
        nbPrises = (uint8_t)(1U + CalculerMaxPrisesDepuis(&copieEtat, caseArrivee));

        if (nbPrises > maximum)
        {
          maximum = nbPrises;
        }
      }
    }

    return maximum;
  }

  if ((typeCase == DAME_BLANCHE) || (typeCase == DAME_NOIRE))
  {
    int32_t directions[4][2] = {
      {-1, -1}, {-1, 1}, {1, -1}, {1, 1}
    };
    uint32_t indexDirection;

    for (indexDirection = 0; indexDirection < 4U; indexDirection++)
    {
      int32_t ligneCourante = (int32_t)depart.ligne + directions[indexDirection][0];
      int32_t colonneCourante = (int32_t)depart.colonne + directions[indexDirection][1];
      uint8_t pieceAdverseTrouvee = 0U;
      PositionCase caseCapturee = {0};

      while (CoordonneesSontDansPlateau(ligneCourante, colonneCourante) != 0U)
      {
        if (etat->plateau[ligneCourante][colonneCourante] == CASE_VIDE)
        {
          if (pieceAdverseTrouvee != 0U)
          {
            PositionCase caseArrivee;
            EtatPartie copieEtat;
            uint8_t nbPrises;

            caseArrivee.ligne = (uint8_t)ligneCourante;
            caseArrivee.colonne = (uint8_t)colonneCourante;

            copieEtat = *etat;
            EffectuerPriseSimple(&copieEtat, depart, caseArrivee, caseCapturee);
            nbPrises = (uint8_t)(1U + CalculerMaxPrisesDepuis(&copieEtat, caseArrivee));

            if (nbPrises > maximum)
            {
              maximum = nbPrises;
            }
          }
        }
        else if (CaseContientPieceAdverse(etat, (uint8_t)ligneCourante, (uint8_t)colonneCourante) != 0U)
        {
          if (pieceAdverseTrouvee != 0U)
          {
            break;
          }

          pieceAdverseTrouvee = 1U;
          caseCapturee.ligne = (uint8_t)ligneCourante;
          caseCapturee.colonne = (uint8_t)colonneCourante;
        }
        else
        {
          break;
        }

        ligneCourante += directions[indexDirection][0];
        colonneCourante += directions[indexDirection][1];
      }
    }
  }

  return maximum;
}

static uint8_t CalculerMaxPrisesJoueur(const EtatPartie *etat)
{
  uint8_t maximum = 0U;
  uint32_t ligne;
  uint32_t colonne;

  for (ligne = 0; ligne < TAILLE_PLATEAU; ligne++)
  {
    for (colonne = 0; colonne < TAILLE_PLATEAU; colonne++)
    {
      if (CaseContientPieceDuJoueur(etat, (uint8_t)ligne, (uint8_t)colonne) != 0U)
      {
        PositionCase position;
        uint8_t nbPrises;

        position.ligne = (uint8_t)ligne;
        position.colonne = (uint8_t)colonne;
        nbPrises = CalculerMaxPrisesDepuis(etat, position);

        if (nbPrises > maximum)
        {
          maximum = nbPrises;
        }
      }
    }
  }

  return maximum;
}

static uint8_t PriseRespecteLeMaximum(const EtatPartie *etat, PositionCase depart, PositionCase arrivee, PositionCase *caseCapturee)
{
  PositionCase caseCaptureeLocale;
  EtatPartie copieEtat;
  uint8_t maximumDepuisDepart;
  uint8_t nbPrises;

  if (PriseSimpleEstValide(etat, depart, arrivee, &caseCaptureeLocale) == 0U)
  {
    return 0U;
  }

  copieEtat = *etat;
  EffectuerPriseSimple(&copieEtat, depart, arrivee, caseCaptureeLocale);

  maximumDepuisDepart = CalculerMaxPrisesDepuis(etat, depart);
  nbPrises = (uint8_t)(1U + CalculerMaxPrisesDepuis(&copieEtat, arrivee));

  if (nbPrises != maximumDepuisDepart)
  {
    return 0U;
  }

  *caseCapturee = caseCaptureeLocale;
  return 1U;
}

static uint8_t PiecePeutCapturerDepuis(const EtatPartie *etat, PositionCase depart)
{
  return (uint8_t)(CalculerMaxPrisesDepuis(etat, depart) != 0U);
}

static uint8_t JoueurDoitCapturer(const EtatPartie *etat)
{
  return (uint8_t)(CalculerMaxPrisesJoueur(etat) != 0U);
}

static uint8_t PiecePeutEtreSelectionnee(const EtatPartie *etat, PositionCase caseTouchee)
{
  uint8_t maximumJoueur;

  if ((CaseEstJouable(caseTouchee.ligne, caseTouchee.colonne) == 0U) ||
      (CaseContientPieceDuJoueur(etat, caseTouchee.ligne, caseTouchee.colonne) == 0U))
  {
    return 0U;
  }

  maximumJoueur = CalculerMaxPrisesJoueur(etat);

  if (maximumJoueur == 0U)
  {
    return 1U;
  }

  return (uint8_t)(CalculerMaxPrisesDepuis(etat, caseTouchee) == maximumJoueur);
}

static uint8_t PiecePeutSeDeplacerDepuis(const EtatPartie *etat, PositionCase depart)
{
  TypeCase typeCase = etat->plateau[depart.ligne][depart.colonne];

  if ((typeCase == PION_BLANC) || (typeCase == PION_NOIR))
  {
    int32_t directionLigne = (typeCase == PION_BLANC) ? 1 : -1;
    int32_t colonneCible;
    int32_t ligneCible;

    ligneCible = (int32_t)depart.ligne + directionLigne;
    if (CoordonneesSontDansPlateau(ligneCible, (int32_t)depart.colonne - 1) != 0U)
    {
      colonneCible = (int32_t)depart.colonne - 1;
      if (CaseEstVide(etat, (uint8_t)ligneCible, (uint8_t)colonneCible) != 0U)
      {
        return 1U;
      }
    }

    if (CoordonneesSontDansPlateau(ligneCible, (int32_t)depart.colonne + 1) != 0U)
    {
      colonneCible = (int32_t)depart.colonne + 1;
      if (CaseEstVide(etat, (uint8_t)ligneCible, (uint8_t)colonneCible) != 0U)
      {
        return 1U;
      }
    }

    return 0U;
  }

  if ((typeCase == DAME_BLANCHE) || (typeCase == DAME_NOIRE))
  {
    int32_t directions[4][2] = {
      {-1, -1}, {-1, 1}, {1, -1}, {1, 1}
    };
    uint32_t indexDirection;

    for (indexDirection = 0; indexDirection < 4U; indexDirection++)
    {
      int32_t ligneCourante = (int32_t)depart.ligne + directions[indexDirection][0];
      int32_t colonneCourante = (int32_t)depart.colonne + directions[indexDirection][1];

      while (CoordonneesSontDansPlateau(ligneCourante, colonneCourante) != 0U)
      {
        if (etat->plateau[ligneCourante][colonneCourante] != CASE_VIDE)
        {
          break;
        }

        return 1U;
      }
    }
  }

  return 0U;
}

static uint8_t JoueurPeutJouer(const EtatPartie *etat, TypeJoueur joueur)
{
  EtatPartie copieEtat;
  uint32_t ligne;
  uint32_t colonne;

  copieEtat = *etat;
  copieEtat.joueurCourant = joueur;
  ReinitialiserCapturesEnCours(&copieEtat);

  if (JoueurDoitCapturer(&copieEtat) != 0U)
  {
    return 1U;
  }

  for (ligne = 0; ligne < TAILLE_PLATEAU; ligne++)
  {
    for (colonne = 0; colonne < TAILLE_PLATEAU; colonne++)
    {
      PositionCase position;

      if (CaseContientPieceDuJoueur(&copieEtat, (uint8_t)ligne, (uint8_t)colonne) == 0U)
      {
        continue;
      }

      position.ligne = (uint8_t)ligne;
      position.colonne = (uint8_t)colonne;

      if (PiecePeutSeDeplacerDepuis(&copieEtat, position) != 0U)
      {
        return 1U;
      }
    }
  }

  return 0U;
}

static void EvaluerFinPartie(EtatPartie *etat)
{
  if (JoueurPeutJouer(etat, etat->joueurCourant) != 0U)
  {
    etat->partieTerminee = 0U;
    return;
  }

  etat->partieTerminee = 1U;
  etat->gagnant = (etat->joueurCourant == JOUEUR_BLANC) ? JOUEUR_NOIR : JOUEUR_BLANC;
}

static void ChangerJoueurCourant(EtatPartie *etat)
{
  etat->joueurCourant = (etat->joueurCourant == JOUEUR_BLANC) ? JOUEUR_NOIR : JOUEUR_BLANC;
}

static void PromouvoirPionSiNecessaire(EtatPartie *etat, PositionCase arrivee)
{
  if ((etat->plateau[arrivee.ligne][arrivee.colonne] == PION_BLANC) &&
      (arrivee.ligne == (TAILLE_PLATEAU - 1U)))
  {
    etat->plateau[arrivee.ligne][arrivee.colonne] = DAME_BLANCHE;
  }
  else if ((etat->plateau[arrivee.ligne][arrivee.colonne] == PION_NOIR) &&
           (arrivee.ligne == 0U))
  {
    etat->plateau[arrivee.ligne][arrivee.colonne] = DAME_NOIRE;
  }
}

static void DeplacerPiece(EtatPartie *etat, PositionCase depart, PositionCase arrivee)
{
  etat->plateau[arrivee.ligne][arrivee.colonne] = etat->plateau[depart.ligne][depart.colonne];
  etat->plateau[depart.ligne][depart.colonne] = CASE_VIDE;
}

static void EffectuerPriseSimple(EtatPartie *etat, PositionCase depart, PositionCase arrivee, PositionCase caseCapturee)
{
  DeplacerPiece(etat, depart, arrivee);
  etat->piecesCaptureesEnCours[caseCapturee.ligne][caseCapturee.colonne] = 1U;
}

static void RetirerPiecesCaptureesEnCours(EtatPartie *etat)
{
  uint32_t ligne;
  uint32_t colonne;

  for (ligne = 0; ligne < TAILLE_PLATEAU; ligne++)
  {
    for (colonne = 0; colonne < TAILLE_PLATEAU; colonne++)
    {
      if (etat->piecesCaptureesEnCours[ligne][colonne] != 0U)
      {
        etat->plateau[ligne][colonne] = CASE_VIDE;
      }
    }
  }
}

static void FinaliserTourApresCapture(EtatPartie *etat, PositionCase arrivee)
{
  RetirerPiecesCaptureesEnCours(etat);
  PromouvoirPionSiNecessaire(etat, arrivee);
  FinaliserCoupLocal(etat);
  DeselectionnerCase(etat);
  ChangerJoueurCourant(etat);
  ReinitialiserCapturesEnCours(etat);
  EvaluerFinPartie(etat);
}

static void FinaliserTourSansCapture(EtatPartie *etat, PositionCase arrivee)
{
  PromouvoirPionSiNecessaire(etat, arrivee);
  FinaliserCoupLocal(etat);
  DeselectionnerCase(etat);
  ChangerJoueurCourant(etat);
  ReinitialiserCapturesEnCours(etat);
  EvaluerFinPartie(etat);
}

static void DeselectionnerCase(EtatPartie *etat)
{
  etat->selectionActive = 0U;
  etat->priseMultipleActive = 0U;
  etat->caseSelectionnee.ligne = 0U;
  etat->caseSelectionnee.colonne = 0U;
}

static void SelectionnerCase(EtatPartie *etat, uint8_t ligne, uint8_t colonne)
{
  etat->selectionActive = 1U;
  etat->caseSelectionnee.ligne = ligne;
  etat->caseSelectionnee.colonne = colonne;
}

static void DessinerPlateau(void)
{
  uint32_t ligne;
  uint32_t colonne;
  uint16_t x;
  uint16_t y;

  BSP_LCD_SelectLayer(0);
  BSP_LCD_Clear(COULEUR_FOND_ECRAN);

  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
  BSP_LCD_FillRect(PLATEAU_X - EPAISSEUR_BORDURE_PLATEAU,
                   PLATEAU_Y - EPAISSEUR_BORDURE_PLATEAU,
                   TAILLE_PIXEL_PLATEAU + (2U * EPAISSEUR_BORDURE_PLATEAU),
                   TAILLE_PIXEL_PLATEAU + (2U * EPAISSEUR_BORDURE_PLATEAU));

  for (ligne = 0; ligne < TAILLE_PLATEAU; ligne++)
  {
    for (colonne = 0; colonne < TAILLE_PLATEAU; colonne++)
    {
      x = PLATEAU_X + (uint16_t)(colonne * TAILLE_CASE);
      y = PLATEAU_Y + (uint16_t)(ligne * TAILLE_CASE);

      BSP_LCD_SetTextColor(((ligne + colonne) % 2U) == 0U ? COULEUR_CASE_CLAIRE : COULEUR_CASE_FONCEE);
      BSP_LCD_FillRect(x, y, TAILLE_CASE, TAILLE_CASE);
    }
  }
}

static void ObtenirCentreCase(uint32_t ligne, uint32_t colonne, uint16_t *x, uint16_t *y)
{
  *x = (uint16_t)(PLATEAU_X + (colonne * TAILLE_CASE) + (TAILLE_CASE / 2U));
  *y = (uint16_t)(PLATEAU_Y + (ligne * TAILLE_CASE) + (TAILLE_CASE / 2U));
}

static void AfficherTexteCentreZone(uint16_t x, uint16_t y, uint16_t largeur, const char *texte)
{
  sFONT *policeCourante;
  uint16_t largeurTexte;
  uint16_t xTexte;

  policeCourante = BSP_LCD_GetFont();
  largeurTexte = (uint16_t)(strlen(texte) * policeCourante->Width);

  if (largeurTexte >= largeur)
  {
    xTexte = x;
  }
  else
  {
    xTexte = (uint16_t)(x + ((largeur - largeurTexte) / 2U));
  }

  BSP_LCD_DisplayStringAt(xTexte, y, (uint8_t *)texte, LEFT_MODE);
}

static void DessinerPion(uint32_t ligne, uint32_t colonne, uint32_t couleurRemplissage, uint32_t couleurContour)
{
  uint16_t centreX;
  uint16_t centreY;

  ObtenirCentreCase(ligne, colonne, &centreX, &centreY);

  BSP_LCD_SetTextColor(couleurRemplissage);
  BSP_LCD_FillCircle(centreX, centreY, RAYON_PION);
  BSP_LCD_SetTextColor(couleurContour);
  BSP_LCD_DrawCircle(centreX, centreY, RAYON_PION);
}

static void DessinerSymboleDame(uint32_t ligne, uint32_t colonne, uint32_t couleurSymbole)
{
  uint16_t centreX;
  uint16_t centreY;

  ObtenirCentreCase(ligne, colonne, &centreX, &centreY);

  BSP_LCD_SetTextColor(couleurSymbole);
  BSP_LCD_DrawLine((uint16_t)(centreX - RAYON_SYMBOLE_DAME), centreY,
                   (uint16_t)(centreX + RAYON_SYMBOLE_DAME), centreY);
  BSP_LCD_DrawLine(centreX, (uint16_t)(centreY - RAYON_SYMBOLE_DAME),
                   centreX, (uint16_t)(centreY + RAYON_SYMBOLE_DAME));
  BSP_LCD_DrawLine((uint16_t)(centreX - RAYON_SYMBOLE_DAME), (uint16_t)(centreY - RAYON_SYMBOLE_DAME),
                   (uint16_t)(centreX + RAYON_SYMBOLE_DAME), (uint16_t)(centreY + RAYON_SYMBOLE_DAME));
  BSP_LCD_DrawLine((uint16_t)(centreX - RAYON_SYMBOLE_DAME), (uint16_t)(centreY + RAYON_SYMBOLE_DAME),
                   (uint16_t)(centreX + RAYON_SYMBOLE_DAME), (uint16_t)(centreY - RAYON_SYMBOLE_DAME));
}

static void DessinerPions(const EtatPartie *etat)
{
  uint32_t ligne;
  uint32_t colonne;

  BSP_LCD_SelectLayer(1);

  for (ligne = 0; ligne < TAILLE_PLATEAU; ligne++)
  {
    for (colonne = 0; colonne < TAILLE_PLATEAU; colonne++)
    {
      switch (etat->plateau[ligne][colonne])
      {
        case PION_BLANC:
          DessinerPion(ligne, colonne, COULEUR_PION_BLANC, COULEUR_CONTOUR_PION_BLANC);
          break;

        case PION_NOIR:
          DessinerPion(ligne, colonne, COULEUR_PION_NOIR, COULEUR_CONTOUR_PION_NOIR);
          break;

        case DAME_BLANCHE:
          DessinerPion(ligne, colonne, COULEUR_PION_BLANC, COULEUR_CONTOUR_PION_BLANC);
          DessinerSymboleDame(ligne, colonne, COULEUR_PION_NOIR);
          break;

        case DAME_NOIRE:
          DessinerPion(ligne, colonne, COULEUR_PION_NOIR, COULEUR_CONTOUR_PION_NOIR);
          DessinerSymboleDame(ligne, colonne, COULEUR_PION_BLANC);
          break;

        case CASE_VIDE:
        default:
          break;
      }
    }
  }
}

static void DessinerSelection(const EtatPartie *etat)
{
  uint16_t x;
  uint16_t y;

  if (etat->selectionActive == 0U)
  {
    return;
  }

  x = (uint16_t)(PLATEAU_X + (etat->caseSelectionnee.colonne * TAILLE_CASE));
  y = (uint16_t)(PLATEAU_Y + (etat->caseSelectionnee.ligne * TAILLE_CASE));

  BSP_LCD_SetTextColor(COULEUR_SELECTION_CASE);
  BSP_LCD_DrawRect(x, y, TAILLE_CASE, TAILLE_CASE);
  BSP_LCD_DrawRect((uint16_t)(x + 1U), (uint16_t)(y + 1U), TAILLE_CASE - 2U, TAILLE_CASE - 2U);
}

static void DessinerBoutonQuitter(void)
{
  BSP_LCD_SetTextColor(COULEUR_BOUTON_QUITTER);
  BSP_LCD_FillRect(BOUTON_QUITTER_X, BOUTON_QUITTER_Y, BOUTON_QUITTER_LARGEUR, BOUTON_QUITTER_HAUTEUR);

  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
  BSP_LCD_DrawRect(BOUTON_QUITTER_X, BOUTON_QUITTER_Y, BOUTON_QUITTER_LARGEUR, BOUTON_QUITTER_HAUTEUR);

  BSP_LCD_SetFont(&Font12);
  BSP_LCD_SetTextColor(COULEUR_TEXTE_QUITTER);
  BSP_LCD_SetBackColor(COULEUR_BOUTON_QUITTER);
  AfficherTexteCentreZone(BOUTON_QUITTER_X, (uint16_t)(BOUTON_QUITTER_Y + 10U), BOUTON_QUITTER_LARGEUR, "Quitter");
}

static void DessinerInfosJeu(const EtatPartie *etat)
{
  char texte[40];
  char texteJoueurLocal[40];
  char texteEtat[40];

  BSP_LCD_SetFont(&Font12);
  BSP_LCD_SetTextColor(COULEUR_INFOS_JEU);
  BSP_LCD_SetBackColor(0x00000000);

  if (etat->partieTerminee != 0U)
  {
    snprintf(texte, sizeof(texte), "Victoire : %s", etat->gagnant == JOUEUR_BLANC ? "blanc" : "noir");
  }
  else
  {
    snprintf(texte, sizeof(texte), "Tour : %s", etat->joueurCourant == JOUEUR_BLANC ? "blanc" : "noir");
  }

  BSP_LCD_DisplayStringAt(280, 24, (uint8_t *)texte, LEFT_MODE);

  if (etat->modePartie == DAMES_MODE_UART)
  {
    snprintf(texteJoueurLocal, sizeof(texteJoueurLocal), "Vous : %s",
             etat->joueurLocal == JOUEUR_BLANC ? "blanc" : "noir");
    BSP_LCD_DisplayStringAt(280, 42, (uint8_t *)texteJoueurLocal, LEFT_MODE);

    snprintf(texteEtat, sizeof(texteEtat), "Etat : %s",
             JoueurLocalPeutJouer(etat) != 0U ? "a vous" : "attente");
    BSP_LCD_DisplayStringAt(280, 60, (uint8_t *)texteEtat, LEFT_MODE);
  }
}

static void DessinerElementsJeu(const EtatPartie *etat)
{
  BSP_LCD_SelectLayer(1);
  BSP_LCD_Clear(0x00000000);

  DessinerPions(etat);
  DessinerSelection(etat);
  DessinerInfosJeu(etat);
  DessinerBoutonQuitter();
}
