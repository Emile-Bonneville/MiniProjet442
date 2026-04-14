#include "menu.h"

#include "string.h"
#include "stm32746g_discovery_lcd.h"

#define COULEUR_ACCUEIL_FOND   ((uint32_t)0xFFF8F3EA)
#define COULEUR_CARTE_JEU      ((uint32_t)0xFFE4D1B0)
#define COULEUR_CARTE_BORDURE  ((uint32_t)0xFF7B4F29)
#define COULEUR_TITRE_ACCUEIL  ((uint32_t)0xFF4F2F1A)

#define COULEUR_BOUTON_RETOUR  ((uint32_t)0xFFD8C3A2)

#define CARTE_JEU_X            120U
#define CARTE_JEU_Y            70U
#define CARTE_JEU_LARGEUR      240U
#define CARTE_JEU_HAUTEUR      68U

#define CARTE_MODE_X           100U
#define CARTE_MODE_LARGEUR     280U
#define CARTE_MODE_HAUTEUR     54U
#define CARTE_MODE_LOCAL_Y     78U
#define CARTE_MODE_UART_Y      144U
#define BOUTON_RETOUR_X        160U
#define BOUTON_RETOUR_Y        214U
#define BOUTON_RETOUR_LARGEUR  160U
#define BOUTON_RETOUR_HAUTEUR  36U

typedef enum
{
  MENU_ECRAN_ACCUEIL = 0,
  MENU_ECRAN_DAMES_MODE,
  MENU_ECRAN_DAMES_UART_JOUEUR
} TypeEcranMenu;

static TypeEcranMenu ecranMenuCourant = MENU_ECRAN_ACCUEIL;

static uint8_t CoordonneesSontDansZone(uint16_t x, uint16_t y,
                                       uint16_t zoneX, uint16_t zoneY,
                                       uint16_t largeur, uint16_t hauteur);
static void AfficherTexteCentreZone(uint16_t x, uint16_t y, uint16_t largeur, const char *texte);
static void DessinerCarte(uint16_t x, uint16_t y, uint16_t largeur, uint16_t hauteur,
                          uint32_t couleurFond, const char *titre, const char *sousTitre);
static void AfficherAccueilPrincipal(void);
static void AfficherSousMenuDames(void);
static void AfficherSousMenuDamesUart(void);

void Menu_Reinitialiser(void)
{
  ecranMenuCourant = MENU_ECRAN_ACCUEIL;
}

void Menu_Afficher(void)
{
  if (ecranMenuCourant == MENU_ECRAN_DAMES_MODE)
  {
    AfficherSousMenuDames();
  }
  else if (ecranMenuCourant == MENU_ECRAN_DAMES_UART_JOUEUR)
  {
    AfficherSousMenuDamesUart();
  }
  else
  {
    AfficherAccueilPrincipal();
  }
}

MenuAction Menu_GererTouch(uint16_t x, uint16_t y)
{
  if (ecranMenuCourant == MENU_ECRAN_ACCUEIL)
  {
    if (CoordonneesSontDansZone(x, y, CARTE_JEU_X, CARTE_JEU_Y, CARTE_JEU_LARGEUR, CARTE_JEU_HAUTEUR) != 0U)
    {
      ecranMenuCourant = MENU_ECRAN_DAMES_MODE;
      Menu_Afficher();
      return MENU_ACTION_AUCUNE;
    }

    return MENU_ACTION_AUCUNE;
  }

  if (ecranMenuCourant == MENU_ECRAN_DAMES_MODE)
  {
    if (CoordonneesSontDansZone(x, y, CARTE_MODE_X, CARTE_MODE_LOCAL_Y, CARTE_MODE_LARGEUR, CARTE_MODE_HAUTEUR) != 0U)
    {
      return MENU_ACTION_LANCER_DAMES_LOCAL;
    }

    if (CoordonneesSontDansZone(x, y, CARTE_MODE_X, CARTE_MODE_UART_Y, CARTE_MODE_LARGEUR, CARTE_MODE_HAUTEUR) != 0U)
    {
      ecranMenuCourant = MENU_ECRAN_DAMES_UART_JOUEUR;
      Menu_Afficher();
      return MENU_ACTION_AUCUNE;
    }
  }
  else if (ecranMenuCourant == MENU_ECRAN_DAMES_UART_JOUEUR)
  {
    if (CoordonneesSontDansZone(x, y, CARTE_MODE_X, CARTE_MODE_LOCAL_Y, CARTE_MODE_LARGEUR, CARTE_MODE_HAUTEUR) != 0U)
    {
      return MENU_ACTION_LANCER_DAMES_UART_BLANC;
    }

    if (CoordonneesSontDansZone(x, y, CARTE_MODE_X, CARTE_MODE_UART_Y, CARTE_MODE_LARGEUR, CARTE_MODE_HAUTEUR) != 0U)
    {
      return MENU_ACTION_LANCER_DAMES_UART_NOIR;
    }
  }

  if (CoordonneesSontDansZone(x, y, BOUTON_RETOUR_X, BOUTON_RETOUR_Y, BOUTON_RETOUR_LARGEUR, BOUTON_RETOUR_HAUTEUR) != 0U)
  {
    if (ecranMenuCourant == MENU_ECRAN_DAMES_UART_JOUEUR)
    {
      ecranMenuCourant = MENU_ECRAN_DAMES_MODE;
    }
    else
    {
      ecranMenuCourant = MENU_ECRAN_ACCUEIL;
    }
    Menu_Afficher();
  }

  return MENU_ACTION_AUCUNE;
}

static void AfficherAccueilPrincipal(void)
{
  BSP_LCD_SelectLayer(0);
  BSP_LCD_Clear(COULEUR_ACCUEIL_FOND);

  BSP_LCD_SetFont(&Font24);
  BSP_LCD_SetTextColor(COULEUR_TITRE_ACCUEIL);
  BSP_LCD_SetBackColor(COULEUR_ACCUEIL_FOND);
  AfficherTexteCentreZone(0, 24, (uint16_t)BSP_LCD_GetXSize(), "JEUX");

  DessinerCarte(CARTE_JEU_X, CARTE_JEU_Y, CARTE_JEU_LARGEUR, CARTE_JEU_HAUTEUR,
                COULEUR_CARTE_JEU, "Jeu de dames", "Choisir un mode");

  BSP_LCD_SelectLayer(1);
  BSP_LCD_Clear(0x00000000);
}

static void AfficherSousMenuDames(void)
{
  BSP_LCD_SelectLayer(0);
  BSP_LCD_Clear(COULEUR_ACCUEIL_FOND);

  BSP_LCD_SetFont(&Font24);
  BSP_LCD_SetTextColor(COULEUR_TITRE_ACCUEIL);
  BSP_LCD_SetBackColor(COULEUR_ACCUEIL_FOND);
  AfficherTexteCentreZone(0, 20, (uint16_t)BSP_LCD_GetXSize(), "Jeu de dames");

  DessinerCarte(CARTE_MODE_X, CARTE_MODE_LOCAL_Y, CARTE_MODE_LARGEUR, CARTE_MODE_HAUTEUR,
                COULEUR_CARTE_JEU, "1 carte", "Deux joueurs sur le meme ecran");
  DessinerCarte(CARTE_MODE_X, CARTE_MODE_UART_Y, CARTE_MODE_LARGEUR, CARTE_MODE_HAUTEUR,
                COULEUR_CARTE_JEU, "2 cartes UART", "Une carte par joueur");

  DessinerCarte(BOUTON_RETOUR_X, BOUTON_RETOUR_Y, BOUTON_RETOUR_LARGEUR, BOUTON_RETOUR_HAUTEUR,
                COULEUR_BOUTON_RETOUR, "Retour", NULL);

  BSP_LCD_SelectLayer(1);
  BSP_LCD_Clear(0x00000000);
}

static void AfficherSousMenuDamesUart(void)
{
  BSP_LCD_SelectLayer(0);
  BSP_LCD_Clear(COULEUR_ACCUEIL_FOND);

  BSP_LCD_SetFont(&Font24);
  BSP_LCD_SetTextColor(COULEUR_TITRE_ACCUEIL);
  BSP_LCD_SetBackColor(COULEUR_ACCUEIL_FOND);
  AfficherTexteCentreZone(0, 20, (uint16_t)BSP_LCD_GetXSize(), "2 cartes UART");

  DessinerCarte(CARTE_MODE_X, CARTE_MODE_LOCAL_Y, CARTE_MODE_LARGEUR, CARTE_MODE_HAUTEUR,
                COULEUR_CARTE_JEU, "Joueur blanc", "Cette carte joue les blancs");
  DessinerCarte(CARTE_MODE_X, CARTE_MODE_UART_Y, CARTE_MODE_LARGEUR, CARTE_MODE_HAUTEUR,
                COULEUR_CARTE_JEU, "Joueur noir", "Cette carte joue les noirs");

  DessinerCarte(BOUTON_RETOUR_X, BOUTON_RETOUR_Y, BOUTON_RETOUR_LARGEUR, BOUTON_RETOUR_HAUTEUR,
                COULEUR_BOUTON_RETOUR, "Retour", NULL);

  BSP_LCD_SelectLayer(1);
  BSP_LCD_Clear(0x00000000);
}

static void DessinerCarte(uint16_t x, uint16_t y, uint16_t largeur, uint16_t hauteur,
                          uint32_t couleurFond, const char *titre, const char *sousTitre)
{
  BSP_LCD_SetTextColor(COULEUR_CARTE_BORDURE);
  BSP_LCD_FillRect(x, y, largeur, hauteur);

  BSP_LCD_SetTextColor(couleurFond);
  BSP_LCD_FillRect((uint16_t)(x + 3U), (uint16_t)(y + 3U),
                   (uint16_t)(largeur - 6U), (uint16_t)(hauteur - 6U));

  BSP_LCD_SetTextColor(COULEUR_TITRE_ACCUEIL);
  BSP_LCD_SetBackColor(couleurFond);
  BSP_LCD_SetFont(&Font16);
  AfficherTexteCentreZone(x, (uint16_t)(y + 10U), largeur, titre);

  if (sousTitre != NULL)
  {
    BSP_LCD_SetFont(&Font12);
    AfficherTexteCentreZone(x, (uint16_t)(y + 30U), largeur, sousTitre);
  }
}

static uint8_t CoordonneesSontDansZone(uint16_t x, uint16_t y,
                                       uint16_t zoneX, uint16_t zoneY,
                                       uint16_t largeur, uint16_t hauteur)
{
  return (uint8_t)((x >= zoneX) && (x < (zoneX + largeur)) &&
                   (y >= zoneY) && (y < (zoneY + hauteur)));
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
