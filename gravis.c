/************
 * gravis.c *
 ************
 * Copyright (C) 2003 Peter Kolb
 * pekoli@gmail.com 
 *
 * Dieses Programm ist freie Software. Sie können es unter den Bedingungen
 * der GNU General Public License, wie von der Free Software Foundation
 * veröffentlicht, weitergeben und/oder modifizieren, entweder gemäß Version
 * 2 der Lizenz oder (nach Ihrer Option) jeder späteren Version.
 *
 * Die Veröffentlichung dieses Programms erfolgt in der Hoffnung, daß es
 * Ihnen von Nutzen sein wird, aber OHNE IRGENDEINE GARANTIE, sogar ohne die
 * implizite Garantie der MARKTREIFE oder der VERWENDBARKEIT FÜR EINEN
 * BESTIMMTEN ZWECK. Details finden Sie in der GNU General Public License.
 *
 * Sie sollten ein Exemplar der GNU General Public License zusammen mit
 * diesem Programm erhalten haben. Falls nicht, schreiben Sie an die Free
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110, USA.
 */

/* -----------------------------
 * Aufruf: gravis eingabedatei
 * -----------------------------
 * Liest einen Graphen in Matrix-Darstellung ein und versucht ihn
 * optimal zu zeichnen, d.h. mit moeglichst wenig ueberschneidenden
 * Knoten und Kanten. Dazu wird ein "simulated annealing"-Algorithmus
 * eingesetzt. Die Eingabegraphen duerfen nicht zu gross sein 
 * (nicht mehr als ca. 25-30 Knoten), sonst wird keine vernuenftige 
 * Darstellung mehr erreicht.
 * 
 * Format der Eingabedatei: eine Kante pro Zeile, beschrieben durch die 
 * Namen der beiden Knoten und der Kantenmarkierung (Fliesskomma-Wert):
 *     Knotenname1 Knotenname2 Wert\n
 * Ausgabedatei: fig 3.2
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include "gravis.h"

#define SEP " \t\n"     /* Trennzeichen zwischen Token bei strtok() */

/* eigene globale Definitionen */
struct paar {
    int a;
    int b;
    double sm;
    int typ;
};

struct graphknoten {
    int nr;
    double sm;
    int typ;
    struct graphknoten *next;
};

/* Definitionen aus geometrie.c */
struct point {
    int x;
    int y;
};

struct line {
    struct point p1;
    struct point p2;
};

/* eigene globale Variablen */
double f1a;   /* Abstand zwischen Knoten  40.000 */
double f1b;
double min_a = 50.0; /* Mindestabstand zwischen zwei Knoten */
double f2;    /* Abstand Knoten-Rand      10.000 */
double f3;    /* Kantenlaenge           1/10.000 */
double f4;    /* Anzahl kreuzender Kanten  1.000 */
double d_min = 20.0;
double f5;  /* f5 = f4 * pow(d_min, 2.0); Abstand Knoten - Kante (fine-tuning) */ 
                           /* 3.000 */

char **graph_name;
struct point *graph_koord;
struct graphknoten **graph;
struct graphknoten *o;

/* externe Funktionen */
/* aus geometrie.c */
extern int ccw(struct point, struct point, struct point );
extern int intersect(struct line, struct line);
extern double punktabstand(struct point, struct point);
extern int r_abstand(struct point, int);
extern int l_abstand(struct point);
extern int t_abstand(struct point, int);
extern int b_abstand(struct point);
extern double punkt_kante_abstand(struct point, struct line);
/* aus util.c */
extern int anzahl_grossbuchstaben(char *);
extern char *itoa(int);
extern int runden (double);

/**************************************************/
/* Zufaellige Initialisierung eines Startzustands */
/**************************************************/
/* -- Setze Knoten 0 in die Mitte des Bildes
 * -- Waehle zufaellige Bildkoordinaten fuer alle uebrigen Punkte
 */ 
int setze_startzustand(int xmax, int ymax, int n){

    int i;
    time_t now;
    now = time(NULL);

    /* Knoten 0 in Bildmitte setzen */
    graph_koord[0].x = (int) xmax / 2;
    graph_koord[0].y = (int) ymax / 2;
    
    /* Zufallsgenerator "zufaellig" initialisieren */
    srand( (unsigned) now );

    /* rand() liefert int-Zahl zwischen 0 und RAND_MAX */
    for ( i = 1; i <= n-1; i++ ){
	graph_koord[i].x = runden ( xmax * ((double) rand() / RAND_MAX));
	graph_koord[i].y = runden ( ymax * ((double) rand() / RAND_MAX));
    }

    return 0;
}

/**********************************************************************/
/* Wandle Graphen aus Eingabedatenformat in eigene Datenstrukturen um */
/**********************************************************************/
/* Eingabeformat des Graphen: Liste aus Kanten (char *w1, char *w2, double sm, int typ) 
 */
int init_data(char *dateiname){

    int i, vorhanden_a, vorhanden_b, knoten = 0;
    struct graphknoten *t, *u;
    int graph_name_size, graph_size, kanten = 1, fehler;
    FILE *eingabedatei;
    double sm_f;
    char buffer[256], *cp, zeile[1024], *zp, *w1, *w2, *sm_c;
    cp = buffer;
    zp = zeile;

    /* Eingabedatei oeffnen */
    errno = 0;
    eingabedatei = fopen(dateiname, "r");
    fehler = errno;
    if(fehler){
	printf("Konnte %s nicht oeffnen: ", dateiname);
	printf("%s", strerror(fehler));
	return -1;
    }

    /* Vektor *graph_name mit 100 Feldern initialisieren */
    graph_name_size = 100;
    graph_name = (char **) calloc ( (size_t) (graph_name_size), sizeof(char *) );
    if( graph_name == NULL ){
	printf("INIT_DATA: fehlgeschlagen bei graph_name-Initialisierung!\n");
	return -1;
    }

    /* Vektor **graph mit 100 Feldern initialisieren */
    graph_size = 100;
    graph = (struct graphknoten **) calloc( (size_t) (graph_size), sizeof(struct graphknoten *) ); 
    if( graph == NULL ){
	printf("INIT_DATA: fehlgeschlagen bei graph-Initialisierung!\n");
	return -1;
    }

    /* o initialisieren */
    o = (struct graphknoten *) malloc(sizeof(struct graphknoten));
    if( o == NULL ){
	printf("INIT_DATA: fehlgeschlagen bei Initialisierung von o!\n");
	return -1;
    }
    o->nr = 0; o->sm = 0.0;
    o->typ = 0; o->next = o;

    /* Alle Elemente von graph[] zeigen auf o */
    for(i = 0; i <= graph_size-1; i++) graph[i] = o;

    /* Eingabedatei zeilenweise (kantenweise) einlesen */
    /***************************************************/
    while (fgets(zp, 1024, eingabedatei) != NULL){

	/* Zeile nach Tabs aufspalten */
	w1 = strtok(zeile,SEP);
	w2 = strtok(NULL,SEP);
	sm_c = strtok(NULL,SEP);

	/* "kaputte" Zeilen ueberspringen */
	if ( w1 == NULL || w2 == NULL || sm_c == NULL ){
	    printf("Falsches Format in Eingabedatei Zeile %d! Uebersprungen.\n", kanten);
	    continue;
	}

	/* String *sm_c in Fliesskommazahl umwandeln */
	if ( strcmp(sm_c, "inf") == 0 ){ /* Wert kann "inf" sein ! */
	    sm_f = 50.0;
	}
	else{
	    errno = 0;    
	    sm_f = strtod(sm_c, NULL);
	    fehler = errno;
	    if(fehler){
		printf("Fehler bei strtod(): %s ", sm_c);
		printf("%s", strerror(fehler));
		return 0;
	    }    
	}
	
	/* gelesene Kante speichern */
	/* gibts w1 und w2 schon als Knoten? */
	vorhanden_a = -1; vorhanden_b = -1;
	for( i = 0; i <= knoten-1; i++ ){ /* durchlaufe alle bisherigen Knoten(namen) */
	    if (strcmp(graph_name[i], w1) == 0) vorhanden_a = i;  
	    if (strcmp(graph_name[i], w2) == 0) vorhanden_b = i;  
	}
	if ( vorhanden_a == -1 ){ /* w1 noch nicht als Knoten vorhanden */
	    graph_name[knoten] = (char *) malloc((size_t) strlen(w1)+1);
	    if ( graph_name[knoten] == NULL ){
		printf("INIT_DATA: Fehler bei malloc fuer graph_name, knoten = %d.\n", knoten);
		return -1;
	    }
	    vorhanden_a = knoten;
	    strcpy(graph_name[knoten], w1);
	    knoten++;
	    if ( knoten >= graph_name_size ){
		graph_name_size += 100;
		graph_name = (char **) realloc( graph_name, graph_name_size * sizeof(char *) );
		if( graph_name == NULL ){
		    printf("INIT_DATA: REALLOC: fehlgeschlagen bei graph_name (1)!\n");
		    return -1;
		}
	    }   
	}
	if ( vorhanden_b == -1 ){ /* w2 noch nicht als Knoten vorhanden */
	    graph_name[knoten] = (char *) malloc((size_t) strlen(w2)+1);
	    if ( graph_name[knoten] == NULL ){
		printf("INIT_DATA: Fehler bei malloc fuer graph_name, knoten = %d.\n", knoten);
		return -1;
	    }
	    vorhanden_b = knoten;
	    strcpy(graph_name[knoten], w2);
	    knoten++;
	    if ( knoten >= graph_name_size ){
		graph_name_size += 100;
		graph_name = (char **) realloc( graph_name, graph_name_size * sizeof(char *) );
		if( graph_name == NULL ){
		    printf("INIT_DATA: REALLOC: fehlgeschlagen bei graph_name (2)!\n");
		    return -1;
		}
	    }   
	}

	/* In vorhanden_a und vorhanden_b stehen jetzt die Knotennummern der Woerter w1 und w2 */
	/* Vektor graph[] lang genug? */
	if ( knoten >= graph_size ){
	    graph_size += 100;
	    graph = (struct graphknoten **) realloc( graph, graph_size * sizeof(struct graphknoten *) );
	    if( graph == NULL ){
		printf("INIT_DATA: REALLOC: fehlgeschlagen bei graph (2)!\n");
		return -1;
	    }
	}   
	/* Kante in graph[] speichern */
	/* Speicher für neuen Graphknoten allozieren */
	t = (struct graphknoten *) malloc(sizeof(struct graphknoten));
	if( t == NULL ){
	    printf("INIT_DATA:MALLOC: fehlgeschlagen bei t!\n");
	    return -1;
	}
	/* Neuen Knoten initialisieren */
	t->nr = vorhanden_b;
	t->sm = sm_f;
	t->next = o;
	/* Neuen Knoten einbauen */
	u = graph[vorhanden_a];
	graph[vorhanden_a] = t;
	t->next = u;

	/* Kantenzaehler erhoehen */
	kanten++;
    }/* while(fgets...) */

    fclose(eingabedatei);

    /* Vektor graph_koord initialisieren */
    graph_koord = (struct point *) calloc ( (size_t) (knoten+1), sizeof(struct point) );
    if( graph_koord == NULL ){
	printf("INIT_DATA: fehlgeschlagen bei graph_koord-Initialisierung!\n");
	return -1;
    }

    /* Anzahl der Knoten zurueckgeben */
    return knoten;
}


/* Generierungsregel fuer neuen Zustand
 * ------------------------------------
 * Verschiebe einen Knoten k auf eine Kreislinie mit Radius r(t) 
 * um die alten Koordinaten 
 */
void verschiebe_knoten(int k, double t, double t0, int xmax, int ymax, int *zx, int *zy){ 
    double alpha, vb;
    int dx, dy, x2, y2, max_k;
    double pi = 3.14159265358979323846;

    /* verschiebe Knoten k um einen Vektor v */

    /* |v| = r(t) */
    /* r(t0) = max(xmax, ymax) */
    if ( xmax > ymax ){
	max_k = xmax;
    }
    else{
	max_k = ymax;
    }
    
    vb = t / t0 * max_k;
    
    /* Richtung von v zufaellig bestimmen */
    /* Einen zufaelligen Winkel [0...360] bestimmen */
    alpha = 360 * ( (double) rand() / RAND_MAX);
    /* alpha ins Bogenmass umrechnen */
    alpha = 2 * pi / 360 * alpha;
    /* Komponenten des Vektors v bestimmen */
    dx = runden (cos (alpha) * vb); 
    dy = runden (sin (alpha) * vb);

    /* neue Koordinaten von Knoten k bestimmen */
    x2 = graph_koord[k].x + dx;
    y2 = graph_koord[k].y + dy;

    /* neue Koordinaten auf Bildgrenzen abschneiden */
    if ( x2 < 0 ) x2 = 10;
    if ( x2 > xmax ) x2 = xmax-10;
    if ( y2 < 0 ) y2 = 10;
    if ( y2 > ymax ) y2 = ymax-10;

    *zx = x2;
    *zy = y2;

    return;
}

/* Anfangskosten 
 * -------------
 * Berechnet die Kosten des zufaellig gewaehlten Startzustands.
 */
double init_kosten(int xmax, int ymax, int n){

    double e = 0;
    int i,j, kij;
    double a, d;
    struct graphknoten *t, *u;
    struct line k1, k2;

    /* Abstand zwischen zwei Knoten */
    /* Fuer jedes Knotenpaar (i,j) addiere zu Kostenfunktion */
    for ( i = 0; i <= n-1; i++ ){
	for ( j = i+1; j <= n-1; j++ ){ /* (i,j) = (j,i). NUR EINMAL!  */
	    /* Gibt es eine Kante zwischen i und j ? */
	    kij = 0;
	    for (t = graph[i]; t != o; t = t->next){
		if ( t->nr == j ){
		    kij = 1;
		    break;
		}
	    }
	    if ( kij == 0 ){
		for (t = graph[j]; t != o; t = t->next){
		    if ( t->nr == i ){
			kij = 1;
			break;
		    }
		}
	    }

	    /* Wenn es keine Kante zwischen i und j gibt (sollen sich abstossen) */
	    a = punktabstand(graph_koord[i], graph_koord[j]);
	    if ( (kij == 0) || (a < min_a) ){
		e = e + f1a / pow ( a, 2.0);
	    /* Wenn es eine Kante zwischen i und j gibt sollen sie sich anziehen */
	    /* (bis zum Minimalabstand min_a=20.0) */
	    }else{
		e = e + f1b * pow ( a, 2.0);
	    }
	}
    }

    /* Knoten innerhalb des Bildes */
    /* Fuer jeden Knoten i addiere zu Kostenfunktion */
    for ( i = 0; i <= n-1; i++ ){
	e = e + f2 * ( 1/pow(r_abstand(graph_koord[i], xmax), 2.0) + 
		       1/pow(l_abstand(graph_koord[i])      , 2.0) + 
		       1/pow(t_abstand(graph_koord[i], ymax), 2.0) + 
		       1/pow(b_abstand(graph_koord[i])      , 2.0) );
    }

    /* minimale Kantenlaengen */
    /* Fuer jede Kante in graph[] addiere */
    for ( i = 0; i <= n-1; i++ ){
	for ( t = graph[i]; t != o; t = t->next ){
	    e = e + f3 * pow ( punktabstand(graph_koord[i], graph_koord[t->nr]), 2.0);
	}
    }

    /* Anzahl sich kreuzender Kanten */
    /* Fuer jedes Kantenpaar in graph[] addiere */
    for ( i = 0; i <= n-1; i++ ){
	for ( t = graph[i]; t != o; t = t->next ){
	    if ( t != o ){
		j = i; u = t->next;
		do{
		    while ( u != o ){
			/* Kante 1: graph_koord[i], graph_koord[t->nr] */
			k1.p1.x = graph_koord[i].x;
			k1.p1.y = graph_koord[i].y;
			k1.p2.x = graph_koord[t->nr].x;
			k1.p2.y = graph_koord[t->nr].y;
			/* Kante 2: graph_koord[j], graph_koord[u->nr] */
			k2.p1.x = graph_koord[j].x;
			k2.p1.y = graph_koord[j].y;
			k2.p2.x = graph_koord[u->nr].x;
			k2.p2.y = graph_koord[u->nr].y;
			/* Schneiden sich k1 und k2 ? */
			e = e + f4 * intersect( k1, k2 );
			
			u = u->next;
		    }
		    j++; u = graph[j];
		}while( j <= n-1 );
	    }
	}
    }

    /* Abstand Knoten-Kante (fine tuning) */
    for(j = 0; j <= n-1; j++){
	for ( i = 0; i <= n-1; i++ ){
	    for ( t = graph[i]; t != o; t = t->next ){
		if ( (t != o) && (i != j) && (t->nr != j) ){
		    k1.p1.x = graph_koord[i].x;
		    k1.p1.y = graph_koord[i].y;
		    k1.p2.x = graph_koord[t->nr].x;
		    k1.p2.y = graph_koord[t->nr].y;
		    d = punkt_kante_abstand(graph_koord[j], k1);
		    if ( d < d_min ){
			e = e + f4;
		    }
		    else{
			e = e + f5 * 1/pow(d, 2.0);
		    }
		}
	    }
	}
    }

    return e;
}

/*********************************************************************/
/* Berechnet die neuen Kosten ek_neu, wenn der Knoten k              */
/* auf seine neue Position (x_neu, y_neu) verschoben wird.           */
/*********************************************************************/
double kostendifferenz(int xmax, int ymax, int n, int k, int x_neu, int y_neu){

    double ek_alt = 0, ek_neu = 0, d, kik;
    int a, i, j, x_alt, y_alt;
    struct graphknoten *t, *u;
    struct line k1, k2;

    /* Berechne den Kostenanteil von Knoten k nach den aktuellen (alten) Koordinaten */
    /*********************************************************************************/

    /* Abstand zwischen zwei Knoten */
    /* Fuer jedes Knotenpaar (k,i) addiere zu Kostenfunktion */
    for ( i = 0; i <= n-1; i++ ){
	if ( i == k ) continue;
	/* Gibt es eine Kante zwischen i und k ? */
	kik = 0;
	for (t = graph[i]; t != o; t = t->next){
	    if ( t->nr == k ){
		kik = 1;
		break;
	    }
	}
	if ( kik == 0 ){
	    for (t = graph[k]; t != o; t = t->next){
		if ( t->nr == i ){
		    kik = 1;
		    break;
		}
	    }
	}
	
	/* Wenn es keine Kante zwischen i und k gibt (sollen sich abstossen) */
	a = punktabstand(graph_koord[i], graph_koord[k]);
	if ( (kik == 0) || (a < min_a) ){
	    ek_alt = ek_alt + f1a / pow ( a, 2.0);
	    /* Wenn es eine Kante zwischen i und k gibt sollen sie sich anziehen */
	    /* (bis zum Minimalabstand min_a=20.0) */
	}else{
	    ek_alt = ek_alt + f1b * pow ( a, 2.0);
	}
    }

    /* Knoten innerhalb des Bildes */
    /* Fuer den Knoten k addiere zu Kostenfunktion */
    ek_alt = ek_alt + f2 * ( 1/pow(r_abstand(graph_koord[k], xmax), 2.0) + 
			     1/pow(l_abstand(graph_koord[k])      , 2.0) + 
			     1/pow(t_abstand(graph_koord[k], ymax), 2.0) + 
			     1/pow(b_abstand(graph_koord[k])      , 2.0) );

    /* minimale Kantenlaengen */
    /* Fuer jede Kante mit k in graph[] addiere */
    for ( i = 0; i <= n-1; i++ ){
	for ( t = graph[i]; t != o; t = t->next ){
	    if ( (i == k) || (t->nr == k) ){
		ek_alt = ek_alt + f3 * pow ( punktabstand(graph_koord[i], graph_koord[t->nr]), 2.0);
	    }
	}
    }

    /* Anzahl sich kreuzender Kanten */
    /* Fuer jedes Kantenpaar (k, x) in graph[] addiere */
    for ( i = 0; i <= n-1; i++ ){
	for ( t = graph[i]; t != o; t = t->next ){
	    if ( (i == k) || (t->nr == k) ){
		j = i; u = t->next;
		do{
		    while ( u != o ){
			if ( (j != k) && (u->nr != k) ){
			    /* Kante 1: graph_koord[i], graph_koord[t->nr] */
			    k1.p1.x = graph_koord[i].x;
			    k1.p1.y = graph_koord[i].y;
			    k1.p2.x = graph_koord[t->nr].x;
			    k1.p2.y = graph_koord[t->nr].y;
			    /* Kante 2: graph_koord[j], graph_koord[u->nr] */
			    k2.p1.x = graph_koord[j].x;
			    k2.p1.y = graph_koord[j].y;
			    k2.p2.x = graph_koord[u->nr].x;
			    k2.p2.y = graph_koord[u->nr].y;
			    /* Schneiden sich k1 und k2 ? */
			    ek_alt = ek_alt + f4 * intersect( k1, k2 );
			}
			u = u->next;
		    }
		    j++; u = graph[j];
		}while( j <= n-1 );
	    }
	}
    }

    /* Abstand des Knotens k von allen Kanten, die nicht durch k gehen (nur beim fine-tuning) */
    for ( i = 0; i <= n-1; i++ ){
	for ( t = graph[i]; t != o; t = t->next ){
	    if ( (t != o) && (i != k) && (t->nr != k) ){
		k1.p1.x = graph_koord[i].x;
		k1.p1.y = graph_koord[i].y;
		k1.p2.x = graph_koord[t->nr].x;
		k1.p2.y = graph_koord[t->nr].y;
		d = punkt_kante_abstand(graph_koord[k], k1);
		if ( d < d_min ){
		    ek_alt = ek_alt + f4;
		}
		else{
		    ek_alt = ek_alt + f5 * 1/pow(d, 2.0);
		}
	    }
	}
    }
    
    /* Berechne den Kostenanteil von Knoten k nach den NEUEN Koordinaten */
    /*********************************************************************/
    x_alt = graph_koord[k].x; /* alte Koordinaten merken */
    y_alt = graph_koord[k].y;
    graph_koord[k].x = x_neu; /* k voruebergehend auf neue Koordinaten setzen */
    graph_koord[k].y = y_neu;

    /* Abstand zwischen zwei Knoten */
    /* Fuer jedes Knotenpaar (k,i) addiere zu Kostenfunktion */
    for ( i = 0; i <= n-1; i++ ){
	if ( i == k ) continue;
	/* Gibt es eine Kante zwischen i und k ? */
	kik = 0;
	for (t = graph[i]; t != o; t = t->next){
	    if ( t->nr == k ){
		kik = 1;
		break;
	    }
	}
	if ( kik == 0 ){
	    for (t = graph[k]; t != o; t = t->next){
		if ( t->nr == i ){
		    kik = 1;
		    break;
		}
	    }
	}
	
	/* Wenn es keine Kante zwischen i und k gibt (sollen sich abstossen) */
	a = punktabstand(graph_koord[i], graph_koord[k]);
	if ( (kik == 0) || (a < min_a) ){
	    ek_neu = ek_neu + f1a / pow ( a, 2.0);
	    /* Wenn es eine Kante zwischen i und k gibt sollen sie sich anziehen */
	    /* (bis zum Minimalabstand min_a=20.0) */
	}else{
	    ek_neu = ek_neu + f1b * pow ( a, 2.0);
	}
    }

    /* Knoten innerhalb des Bildes */
    /* Fuer den Knoten k addiere zu Kostenfunktion */
    ek_neu = ek_neu + f2 * ( 1/pow(r_abstand(graph_koord[k], xmax), 2.0) + 
			     1/pow(l_abstand(graph_koord[k])      , 2.0) + 
			     1/pow(t_abstand(graph_koord[k], ymax), 2.0) + 
			     1/pow(b_abstand(graph_koord[k])      , 2.0) );

    /* minimale Kantenlaengen */
    /* Fuer jede Kante mit k in graph[] addiere */
    for ( i = 0; i <= n-1; i++ ){
	for ( t = graph[i]; t != o; t = t->next ){
	    if ( (i == k) || (t->nr == k) ){
		ek_neu = ek_neu + f3 * pow ( punktabstand(graph_koord[i], graph_koord[t->nr]), 2.0);
	    }
	}
    }

    /* Anzahl sich kreuzender Kanten */
    /* Fuer jedes Kantenpaar (k, x) in graph[] addiere */
    for ( i = 0; i <= n-1; i++ ){
	for ( t = graph[i]; t != o; t = t->next ){
	    if ( (i == k) || (t->nr == k) ){
		j = i; u = t->next;
		do{
		    while ( u != o ){
			if ( (j != k) && (u->nr != k) ){
			    /* Kante 1: graph_koord[i], graph_koord[t->nr] */
			    k1.p1.x = graph_koord[i].x;
			    k1.p1.y = graph_koord[i].y;
			    k1.p2.x = graph_koord[t->nr].x;
			    k1.p2.y = graph_koord[t->nr].y;
			    /* Kante 2: graph_koord[j], graph_koord[u->nr] */
			    k2.p1.x = graph_koord[j].x;
			    k2.p1.y = graph_koord[j].y;
			    k2.p2.x = graph_koord[u->nr].x;
			    k2.p2.y = graph_koord[u->nr].y;
			    /* Schneiden sich k1 und k2 ? */
			    ek_neu = ek_neu + f4 * intersect( k1, k2 ); 
			}
			u = u->next;
		    }
		    j++; u = graph[j];
		}while( j <= n-1 );
	    }
	}
    }

    /* Abstand des Knotens k von allen Kanten, die nicht durch k gehen */
    for ( i = 0; i <= n-1; i++ ){
	for ( t = graph[i]; t != o; t = t->next ){
	    if ( (t != o) && (i != k) && (t->nr != k) ){
		k1.p1.x = graph_koord[i].x;
		k1.p1.y = graph_koord[i].y;
		k1.p2.x = graph_koord[t->nr].x;
		k1.p2.y = graph_koord[t->nr].y;
		d = punkt_kante_abstand(graph_koord[k], k1);
		if ( d < d_min ){
		    ek_neu = ek_neu + f4;
		}
		else{
		    ek_neu = ek_neu + f5 * 1/pow(d, 2.0);
		}
	    }
	}
    }

    graph_koord[k].x = x_alt; /* k wieder auf alte Koordinaten setzen */
    graph_koord[k].y = y_alt;

    return (ek_alt - ek_neu);
}

/***************************************/
/* Zeichnet den Graphen im xfig-Format */
/***************************************/
int zeichne_graphen(int n, char *dateiname){

    int i, fehler, dicke, wortlaenge, laenge;
    FILE *ausgabedatei;
    char buffer[256];
    struct graphknoten *t;

    /* Ausgabedatei oeffnen */
    strcpy(buffer, dateiname);
    strcat(buffer, ".fig");
    errno = 0;
    ausgabedatei = fopen(buffer, "w");
    fehler = errno;
    if(fehler){
	printf("Konnte %s nicht oeffnen: ", buffer);
	printf("%s", strerror(fehler));
	return -1;
    }

    /* fig-Header schreiben */
    fprintf(ausgabedatei, "#FIG 3.2\n");
    fprintf(ausgabedatei, "Portrait\n");  /* Querformat=Landscape, Hochformat=Portrait */
    fprintf(ausgabedatei, "Center\n");    /* Ausrichtung (Flush Left) */
    fprintf(ausgabedatei, "Metric\n");    /* (Inches) */
    fprintf(ausgabedatei, "A4\n");        /* Papiergroesse (A3) */
    fprintf(ausgabedatei, "100.00\n");    /* Vergroesserung in Prozent */
    fprintf(ausgabedatei, "Single\n");    /* Seitenzahl (multiple) */
    fprintf(ausgabedatei, "-2\n");        /* Transparente Farbe fuer GIF: -2=None */
    fprintf(ausgabedatei, "1200 2\n");    /* Aufloesung (1200ppi); Nullpunkt OBEN links (=2) */

    /* Kanten zeichnen */
    for(i = 0; i <= n-1; i++ ){
	for(t = graph[i]; t != o; t = t->next ){
	    /* Kante von Knoten i zu Knoten t->nr */
	    fprintf(ausgabedatei, "%s ", polyline_h1);
	    dicke = (int) (log10( 150*(t->sm) ) + 1);
	    if ( dicke < 1 ) dicke = 1;
	    fprintf(ausgabedatei, "%d ", dicke);
	    fprintf(ausgabedatei, "%s\n", polyline_h2);
	    fprintf(ausgabedatei, "\t%d %d %d %d\n", graph_koord[i].x*8, graph_koord[i].y*8, graph_koord[t->nr].x*8, graph_koord[t->nr].y*8);
	}
    }

    /* Knoten (Ellipse) zeichnen */
    for(i = 0; i <= n-1; i++ ){
	fprintf(ausgabedatei, "%s ", ellipse_h);	
	fprintf(ausgabedatei, "%d %d ", graph_koord[i].x*8, graph_koord[i].y*8); /* center */
	wortlaenge = strlen(graph_name[i]);
	if ( wortlaenge <= 2 ) wortlaenge = 3;
	laenge = 12 * wortlaenge / 2 + anzahl_grossbuchstaben(graph_name[i])*6;
	fprintf(ausgabedatei, "%d %d ", laenge*8, 150); /* Radien */
	fprintf(ausgabedatei, "%d %d %d %d\n", 11, 22, 33, 44); /* 2 Punkte */	
    }

    /* Knotennamen zeichnen */
    for(i = 0; i <= n-1; i++ ){
	/* Textbox */
	fprintf(ausgabedatei, "%s ", text_h);
	laenge = 12 * strlen(graph_name[i]);
	fprintf(ausgabedatei, "%d ", laenge);
	fprintf(ausgabedatei, "%d %d ", graph_koord[i].x*8, (graph_koord[i].y*8)+67);
	fprintf(ausgabedatei, "%s\\001\n", graph_name[i]);
    }

    fclose(ausgabedatei);

    return 0;
}

/******************
 * Hilfe ausgeben *
 ******************/
void hilfe_ausgeben(){

    printf("Aufruf: gravis <Eingabedatei> [Optionen]\n");
    printf("Format der Eingabedatei: (string) Wort1 (string) wort2 (float) Wert\\n\n");
    printf("Format der Ausgabedatei: fig V3.2\n");
    printf("Expertenoptionen:\n");
    printf("\t-st\tStarttemperatur (default: 150.0)\n");
    printf("\t-gm\tgamma (default: 0.90)\n");
    printf("\t-vt\tKnotenverschiebungen pro Temp. (default: 40)\n");
    printf("\t-at\tAnzahl zu durchlaufender Temp. (default: 60)\n");
    printf("\t-ba\tWahrscheinlichkeitsfaktor \"bergauf\" (default: 0.5)\n");
}

/*****************/
/* HAUPTFUNKTION */
/*****************/
int main(int argc, char *argv[]){

    /* variierbare Parameter -- Defaultwerte */
    double t0 = 150.0; /* Starttemperatur t0 (default=150.0) */
    double gamma = 0.90; /* [0.6 ... 0.95] je kleiner, desto schneller und schlechter (default=0.90) */
    int vpt = 40; /* Anzahl Knotenverschiebungen pro Temperatur (default=30) */
    int at = 60; /* Anzahl zu durchlaufender Temperaturen (default=50) */
    double bergauf_faktor = 0.5; /* bestimmt Haeufigkeit der bergauf-Schritte (je groesser, desto mehr, 
				    default = 0.5 )*/

    int xmax, ymax;
    double t, e, e_diff, e_neu, zufall;
    int temps = 0, i;
    int k, x_neu, y_neu, n; /* n = Anzahl Knoten */
    int bergauf, besser;
    FILE *test;

    /**************************/
    /* Kommandozeile auslesen */
    /**************************/
    printf("GRAVIS V1.0, Copyright (C) 2003 Peter Kolb\n");
    if ( argc < 2 ){
	printf("FEHLER: Keine Eingabedatei!\n");
	hilfe_ausgeben();
	return 0;
    }
    /* existiert die Eingabedatei? */
    errno = 0;
    if ( (test = fopen(argv[1], "r")) == NULL ){
	printf("Konnte \"%s\" nicht oeffnen: ", argv[1]);
	printf("%s\n", strerror(errno));
	hilfe_ausgeben();
	return 0;
    }else{
	fclose(test);
    }
    /* Expertenoptionen */
    if ( argc > 2 ){
	for( i = 2; i <= argc-1; i = i+2 ){
	    if(strcmp(argv[i],"-st") == 0){
		if ( i == argc-1 ){
		    printf("Kein Argument bei Option %s!\n", argv[i]);
		    return 0;
		}
		if ( sscanf(argv[i+1], "%lf", &t0) == EOF ){
		    printf("Ungueltiges Argument bei Option -st: %s", argv[i+1]);
		    return 0;
		}
		printf("Starttemperatur = %f\n", t0);
		continue;
	    }
	    if(strcmp(argv[i],"-gm") == 0){
		if ( i == argc-1 ){
		    printf("Kein Argument bei Option %s!\n", argv[i]);
		    return 0;
		}
		if ( sscanf(argv[i+1], "%lf", &gamma) == EOF ){
		    printf("Ungueltiges Argument bei Option -gm: %s", argv[i+1]);
		    return 0;
		}
		printf("gamma = %f\n", gamma);
		continue;
	    }
	    if(strcmp(argv[i],"-vt") == 0){
		if ( i == argc-1 ){
		    printf("Kein Argument bei Option %s!\n", argv[i]);
		    return 0;
		}
		if ( sscanf(argv[i+1], "%d", &vpt) == EOF ){
		    printf("Ungueltiges Argument bei Option -vt: %s", argv[i+1]);
		    return 0;
		}
		printf("Verschiebungen/Temp. = %d\n", vpt);
		continue;
	    }
	    if(strcmp(argv[i],"-at") == 0){
		if ( i == argc-1 ){
		    printf("Kein Argument bei Option %s!\n", argv[i]);
		    return 0;
		}
		if ( sscanf(argv[i+1], "%d", &at) == EOF ){
		    printf("Ungueltiges Argument bei Option -at: %s", argv[i+1]);
		    return 0;
		}
		printf("Anzahl Temp. = %d\n", at);
		continue;
	    }
	    if(strcmp(argv[i],"-ba") == 0){
		if ( i == argc-1 ){
		    printf("Kein Argument bei Option %s!\n", argv[i]);
		    return 0;
		}
		if ( sscanf(argv[i+1], "%lf", &bergauf_faktor) == EOF ){
		    printf("Ungueltiges Argument bei Option -ba: %s", argv[i+1]);
		    return 0;
		}
		printf("Bergauf-Faktor = %f\n", bergauf_faktor);
		continue;
	    }
	    printf("Unbekannte Option: %s!\n", argv[i]);
	}
    }

    /**********************************/
    /* Datenstrukturen initialisieren */
    /**********************************/
    if ( (n = init_data(argv[1])) < 0 ){
	printf("Fehler in init_data().\n");
	return -1;
    }

    /* Bildgroesse berechnen */
    xmax = 35 * (n+20);
    ymax = runden ( (double) 4.0 * xmax / 3.0);

    /* Faktoren gewichten (so dass alle zunaechst gleichwertig sind) */
    /* f1: Abstand zwischen Knoten, f2: Abstand Knoten-Bildrand, f3: Gesamtlaenge aller
     * Kanten, f4: Anzahl sich kreuzender Kanten, f5: Abstand Knoten-Kanten */
    f1a = pow ( (10 * n), 2.0);
    f1b = 1 / pow ( (10 * n), 2.0);
    f2 = pow ( (10 * n), 2.0);
    f3 = 1 / pow ( (10 * n), 2.0);
    f4 = 1;
    f5 = pow ( (10 * n), 2.0);
    /* bis hierher sind jetzt alle Faktoren gleich wichtig */
    /* folgende zwei Faktoren hoeher gewichten: */
    f4 = f4 * 2;   /* kreuzende Kanten am wichtigsten (daher mal 2) */
    f3 = f3 * 1.5; /* Gesamtlaenge aller Kanten */

    if ( setze_startzustand(xmax, ymax, n) < 0 ){
	printf("Fehler in setze_startzustand().\n");
	return -1;
    }

    t = t0;
    e = init_kosten(xmax, ymax, n);
    
    /* Simulated Annealing */
    do {
	bergauf=0; besser=0;
	for ( i = 1; i <= vpt*n; i++ ){ /* pro Temperatur vpt Knotenverschiebungen */
	    
	    /* waehle neuen Zustand */
            /* zufaellig einen Knoten zwischen 1 und n-1 aussuchen u. virtuell an 
	       neue Koordinaten x_neu, y_neu verschieben */
	    k = runden ( (n-2) * ( (double) rand() / RAND_MAX )) + 1; 
	    verschiebe_knoten(k, t, t0, xmax, ymax, &x_neu, &y_neu);

	    /* berechne neue Kosten falls k an neue Koordinaten verschoben wird */
	    e_diff = kostendifferenz(xmax, ymax, n, k, x_neu, y_neu);
	    e_neu = e - e_diff;

	    /* Zufall (bergauf) */
	    zufall = ( (double) rand() / RAND_MAX); /* zufall zwischen 0 und 1 */
	    if (e_neu <= e){
		/* gehe in neuen Zustand ueber */
		e = e_neu;
		besser++;
		graph_koord[k].x = x_neu; /* k tatsaechlich verschieben */		
		graph_koord[k].y = y_neu;		
	    }
	    else if (zufall < (bergauf_faktor * exp((e - e_neu)/t)) ) {
		e = e_neu;
		bergauf++;
		graph_koord[k].x = x_neu; /* k tatsaechlich verschieben */		
		graph_koord[k].y = y_neu;		
	    }
	}
	/* Temperatur senken */
	t = gamma * t;
	printf("t=%f\te=%f\t(besser:%d, bergauf:%d)\n", t, e, besser, bergauf);
	temps++;
    }while( temps <= at ); /* at Temperaturen durchlaufen */

    /* Ausgabegraphen im xfig-Format zeichnen */
    zeichne_graphen(n, argv[1]);

    return 0;
}

