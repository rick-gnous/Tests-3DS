/**
 * @file main.c
 * @author rick <rick@gnous.eu>
 * date 2021
 * @licence GPL3
 */

#include <citro2d.h>
#include <stdlib.h>
#include <stdio.h>

#define SCREEN_HEIGH 240
#define SCREEN_WIDTH 400

typedef struct
{
  C2D_Sprite spr;
  float x, y;
} sprite ;

void ft_button_press(touchPosition touch, sprite *press, sprite *nopress, sprite *m_blue);

int main(int argc, char *argv[])
{
  /* initialisation du driver RomFS */
  romfsInit(); 
  /* init le framebuffer de l’écran avec des paramètres par défaut */
  gfxInitDefault(); 
  /* init la lib C3D avec la taille du buffer par défaut */
  if (!C3D_Init(C3D_DEFAULT_CMDBUF_SIZE))
    printf("Erreur lors de l’initialisation de C3D.\n");
  /* init la lib C2D avec le max de sprites possible / frame */
  if(!C2D_Init(C2D_DEFAULT_MAX_OBJECTS))
    printf("Erreur lors de l’initialisation de C2D.\n");
  /* prépare le GPU pour du rendu 2D */
  C2D_Prepare();
  /* init la console sur l’écran du bas avec la console actuelle 
   * Ne doit pas être mis si on veut dessiner des sprites sur l’écran */
  /* consoleInit(GFX_BOTTOM, NULL); */

  /* on créé une cible de rendu pour l’écran du haut (on le "créé")
   * On utilise le coté gauche car le droit n’est utilisé qu’en 3D */
  C3D_RenderTarget *top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
  C3D_RenderTarget *bot = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

  /* on récupère la feuille des sprites générée par le Makefile */
  C2D_SpriteSheet sprite_sheet = C2D_SpriteSheetLoad("romfs:/gfx/sprites.t3x");
  if (!sprite_sheet)
    svcBreak(USERBREAK_PANIC);

  /* struct indiquand les coordonnées de l’écran tactile touchées*/
  touchPosition touch; 

  sprite m_blue;
  m_blue.x = 0;
  m_blue.y = 0;
  C2D_SpriteFromSheet(&m_blue.spr, sprite_sheet, 0);
  //C2D_SpriteSetCenter(&m_blue.spr, 0.5f, 0.5f);
  C2D_SpriteSetPos(&m_blue.spr, m_blue.x, m_blue.y);
  //C2D_SpriteSetRotation(&m_blue.spr, C3D_Angle(0.0f));
  
  sprite button_no_press;
  C2D_SpriteFromSheet(&button_no_press.spr, sprite_sheet, 1);
  C2D_SpriteSetCenter(&button_no_press.spr, 0.5f, 0.5f);
  C2D_SpriteSetPos(&button_no_press.spr, 100, 50);

  sprite button_press;
  C2D_SpriteFromSheet(&button_press.spr, sprite_sheet, 2);
  C2D_SpriteSetCenter(&button_press.spr, 0.5f, 0.5f);
  C2D_SpriteSetPos(&button_press.spr, 100, 50);

  while (aptMainLoop())
  {
    /* on capte le boutons pressé */
    hidScanInput();

    /* on récupère le bouton qui est maintenu
     * pour déplacer le sprite */
    u32 bouton_presse = hidKeysHeld(); 

    /* hidKeysHeld retourne l’ensemble des touches pressées.
     * ces opérations bit à bit sont nécessaires pour extraire la touche voulue.*/
    if (bouton_presse & KEY_DOWN)
      /* on récupère la position du sprite (lire la doc pour + d’infos) */
      if (m_blue.spr.params.pos.y < SCREEN_HEIGH)
        /* permet de déplacer le sprite en mettant à jour son x et son y
         * on indique la vitesse de x et y après en pixel/frame (je suppose) */
        C2D_SpriteMove(&m_blue.spr, 0, 5);
    if (bouton_presse & KEY_UP)
      if (m_blue.spr.params.pos.y > 0)
        C2D_SpriteMove(&m_blue.spr, 0, -1);
    if (bouton_presse & KEY_LEFT)
      if (m_blue.spr.params.pos.x > 0)
        C2D_SpriteMove(&m_blue.spr, -1, 0);
    if (bouton_presse & KEY_RIGHT)
      if (m_blue.spr.params.pos.x < SCREEN_WIDTH)
        C2D_SpriteMove(&m_blue.spr, 1, 0);

    /* récpère les coordonnées de l’écran tactile */
    hidTouchRead(&touch);

    /* attend que la frame précédente soit afficher avant de rendre la suivante */
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    /* nettoye l’écran avec la couleur passée en paramètre */
    C2D_TargetClear(top, C2D_Color32f(250.0f, 0.0f, 0.0f, 1.0f));
    /* prépare à écrire sur l’écran top, définit le buffer où écrire */
    C2D_SceneBegin(top);
    /* dessine le sprite */
    C2D_DrawSprite(&m_blue.spr);

    /* on fait la meme pour l’écran du bas */
    C2D_TargetClear(bot, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
    C2D_SceneBegin(bot);
    /* on appelle une fonction pour vérifier si le bouton est pressé */
    ft_button_press(touch, &button_press, &button_no_press, &m_blue);

    /* on indique la fin de la frame */
    C3D_FrameEnd(0);
  }

  /* nécessaire pour nettoyer la mémoire et les buffers */
  C2D_SpriteSheetFree(sprite_sheet);
  C2D_Fini();
  C3D_Fini();
  gfxExit();
  romfsExit();
  return 0;
}

/**
 * ft_button_press(): méthode pour faire tourner le sprite si le bouton est pressé
 * @touch: l’interaction qu’il y a eu sur l’écran tactile
 * @press: le sprite du bouton pressé
 * @nopress: le sprite du bouton non pressé
 * @m_blue: le sprite à faire tourner
 */
void ft_button_press(touchPosition touch, sprite *press, sprite *nopress, sprite *m_blue)
{
  /* on récupère des valeurs très grosses pour simplifier la vérification */
  float heigth = nopress->spr.params.pos.h / 2;
  float weigth = nopress->spr.params.pos.w / 2;
  float sprx = nopress->spr.params.pos.x; 
  float spry = nopress->spr.params.pos.y; 
  if (touch.px >= sprx - weigth && touch.px <= sprx  + weigth &&
      touch.py >= spry - heigth && touch.py <= spry + heigth)
  {
    /* on dessine le bouton pressé et on tourne le sprite */
    C2D_DrawSprite(&press->spr);
    C2D_SpriteRotateDegrees(&m_blue->spr, 5);
  }
  else
    /* on affiche juste le sprite du bouton non pressé */
    C2D_DrawSprite(&nopress->spr);
}
