#include <stdio.h>
#include <stdlib.h> /* rand */
#include <time.h> /* time */
#include <unistd.h> /* pour sleep */

#define PONT 12
#define HOTEL 19
#define PUITS 31
#define LABYRINTHE 42
#define PRISON 52
#define MORT 58
#define JARDIN 63
#define DES1 26
#define DES2 53

#define MAX_JOUEURS 8

/************* Variables globales représentant l'état du jeu ***************/
/* cases occupées par les joueurs */
int positions[MAX_JOUEURS];
int nb_joueurs; /* nombre de joueurs (<= MAX_JOUEURS) */

/* tableau qui gère les différents blocages : */
int est_bloque[MAX_JOUEURS];
/* Les valeurs possibles dans ce tableau sont les suivantes : */
#define HOTEL_1 1 /* Hotel, reste un tour à attendre */
#define HOTEL_2 2 /* Hotel, reste deux tours à attendre */
#define NON_BLOQUE 0
/* Autres valeurs possibles :
 * PUITS s'il est dans le puits
 * PRISON s'il est en prison */

/* vaut 1 si la partie est finie (un joueur a gagné) et 0 sinon. */
int partie_finie;

/************* Fonctions ***************/
/* Initialiser le jeu */
void initialiser_jeu();

/* Retourne un entier uniforme au hasard entre 1 et 6 */
int de();

/* Jouer le tour du joueur numéro joueur
 * TODO améliorer l'intéraction avec l'utilisateur (par exemple avec ncurses) */
void tour(int joueur);

/* Retourne 1 si le joueur est bloqué au début du tour et 0 sinon.
 * En cas de blocage, donne la raison du blocage et met à jour le
 * tableau est_bloque dans le cas où joueur est dans l'hôtel */
int joueur_bloque(int joueur);

/* Étant donné un joueur, une position de depart position et les dés de1 et de2
 * retourne, en suivant les règles de déplacement, la position du joueur à la
 * fin du tour. Vérifie également si joueur a gagné. */
int deplacement(int joueur, int de1, int de2, int position);

#define PAS_COLLISION -1
/* Applique les règles de collision et retourne :
 * un entier j entre 0 et nb_joueurs - 1 s'il y a eu collision avec ce joueur
 * PAS_COLLISION s'il n'y a pas de collision.  */
int collision(int joueur, int position_prec);

/* Applique les règles de blocage à joueur. */
void regle_blocage(int joueur);

/* Affichage rudimentaire du plateau de jeu (sans case départ et avec juste les
 * numéros... 
 * TODO : faire ça mieux avec ncurses. */
void dessiner_plateau();

/* Affiche l'occupant éventuel d'une case (utilisée par dessiner_cases) */
void afficher_occupation(int num_case);

/* Affiche sur deux lignes les cases de premiere à dernière
 * Si derniere < premiere, les cases sont affichée dans l'autre sens */
void dessiner_cases(int premiere, int derniere);

int main()
{
	int j = 0;
	initialiser_jeu();
	while (!partie_finie) {
		tour(j);
		j = (j + 1) % nb_joueurs;
	}
	return 0;
}

void initialiser_jeu()
{
	srand(time(NULL)); /* initialise le générateur de nombres aléatoires */
	/* demander le nombre de joueurs */
	do {
		printf("Nombre de joueurs (entre 1 et %d) ? ", MAX_JOUEURS);
		scanf("%d", &nb_joueurs);
	} while (nb_joueurs < 1 || nb_joueurs > MAX_JOUEURS);
	/* NB : les globales sont initialisées à 0 par défaut donc les tableaux
	 * positions et est_bloque sont déjà correctement initialisées, de même
	 * pour partie_finie */
}

int de()
{
	return (rand() % 6) + 1;
}

int joueur_bloque(int joueur)
{
	if (est_bloque[joueur] == HOTEL_1) {
		printf("Vous êtes bloqué à l'hôtel.\n");
		printf("Vous pourrez rejouer au prochain tour...\n");
		est_bloque[joueur] = NON_BLOQUE;
		return 1;
	}
	if (est_bloque[joueur] == HOTEL_2) {
		printf("Vous êtes bloqué à l'hôtel.\n");
		printf("Vous pourrez rejouer dans deux tours...\n");
		est_bloque[joueur] = HOTEL_1;
		return 1;
	}
	if (est_bloque[joueur] == PRISON) {
		printf("Vous êtes bloqué en prison.\n");
		printf("Il faut attendre qu'un autre joueur vous délivre.\n");
		return 1;
	}
	if (est_bloque[joueur] == PUITS) {
		printf("Vous êtes bloqué dans le puits.\n");
		printf("Il faut attendre qu'un autre joueur prenne votre place\n");
		return 1;
	}
	return 0;
}

int deplacement(int joueur, int de1, int de2, int position)
{
	int encore_un_tour = 0; /* dans le cas des cases de l'oie et d'un
				   dépassement, il faut appliquer de nouveau les
				   règles de déplacement */
	int premier_tour = (position == 0); /* la règle du premier tour s'applique */
	position = position + de1 + de2;
	do {
		encore_un_tour = 0;
		if (premier_tour) {
			if ((de1 == 6 && de2 == 3) || (de1 == 3 && de2 == 6)) {
				position = DES1;
			}
			if ((de1 == 4 && de2 == 5) || (de1 == 5 && de2 == 4)) {
				position = DES2;
			}
		}
		if (position == JARDIN) {
			printf("Le joueur %d est arrivé dans le jardin de l'oie !\n", joueur);
			printf("Victoire pour le joueur %d !\n", joueur);
			partie_finie = 1;
		} else if (position > JARDIN) {
			printf("Vous avez dépassé le jardin !\n");
			printf("Vous reculez de %d cases.\n", position - JARDIN);
			position = JARDIN - (position - JARDIN);
			encore_un_tour = 1;
		} else if (position % 9 == 0) {
			printf("Case de l'oie, vous avancez de nouveau de "
				"%d cases !\n", de1 + de2);
			position = position + de1 + de2;
			encore_un_tour = 1;
		} else if (position == LABYRINTHE) {
			printf("Labyrinthe ! Vous retournez à la case 30 !\n");
			position = 30;
		} else if (position == PONT) {
			printf("Pont ! Vous avancez à la case 12 !\n");
			position = 12;
		} else if (position == MORT) {
			printf("Vous êtes tombé sur la case mort !\n");
			printf("Retour à la case départ...\n");
			position = 0;
		} else {
			printf("Vous avancez à la case %d.\n", position);
		}
	} while (encore_un_tour);
	return position;
}

int collision(int joueur, int position_prec)
{
	int j;
	if (positions[joueur] != 0) {
		for (j = 0; j < nb_joueurs; j = j + 1) {
			if (j != joueur && positions[j] == positions[joueur]) {
				printf("Le joueur %d est sur la même case que vous.\n", j);
				printf("Il retourne à la case %d !\n", position_prec);
				positions[j] = position_prec;
				return j;
			}
		}
	}
	return -1;
}

void regle_blocage(int joueur)
{
	if (positions[joueur] == PUITS) {
		printf("Le joueur %d est tombé dans le puits !\n", joueur);
		est_bloque[joueur] = PUITS;
	} else if (positions[joueur] == PRISON) {
		printf("Le joueur %d est enfermé en prison !\n", joueur);
		est_bloque[joueur] = PRISON;
	} else if (positions[joueur] == HOTEL) {
		printf("Le joueur %d se repose à l'hôtel pendant 2 tours !\n", joueur);
		est_bloque[joueur] = HOTEL_2;
	} else {
		est_bloque[joueur] = 0;
	}
}

void tour(int joueur)
{
	int de1, de2, j, position_prec;
	char c;
	/* Introduction */
	printf("Tour du joueur %d, à la case %d :\n", joueur, positions[joueur]);
	if (joueur_bloque(joueur)) {
		return; /* fin du tour */
	}
	/* Déplacement */
	position_prec = positions[joueur];
	printf("Taper sur une touche puis entrée pour lancer les dés... \n");
	scanf(" %c", &c);
	de1 = de();
	de2 = de();
	printf("Premier dé : %d, deuxième dé : %d\n", de1, de2);
	positions[joueur] = deplacement(joueur, de1, de2, positions[joueur]);
	/* Collision */
	j = collision(joueur, position_prec);
	if (j != PAS_COLLISION) {
		regle_blocage(j);
	}
	/* Blocage */
	regle_blocage(joueur);
	dessiner_plateau();
	printf("Taper sur une touche puis entrée pour continuer... \n");
	scanf(" %c", &c);
}

void afficher_occupation(int num_case)
{
	int j;
	for (j = 0; j < nb_joueurs; j = j + 1) {
		if (positions[j] == num_case) {
			printf("%2d  |", j);
			return;
		}
	}
	printf("    |");
}

void dessiner_cases(int premiere, int derniere)
{
	int i;
	if (premiere < derniere) {
	for (i = premiere; i <= derniere; i = i + 1) {
		printf("  %2d|", i);
	}
	printf("\n");
	for (i = premiere; i <= derniere; i = i + 1) {
		afficher_occupation(i);
	}
	printf("\n");
	} else {
		for (i = premiere; i >= derniere; i = i - 1) {
			printf("  %2d|", i);
		}
		printf("\n");
		for (i = premiere; i >= derniere; i = i - 1) {
			afficher_occupation(i);
		}
		printf("\n");
	}
}

void dessiner_plateau()
{
	dessiner_cases(1, 21);
	dessiner_cases(42, 22);
	dessiner_cases(43, 63);
}
