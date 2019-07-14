/***************
 * geometrie.c *
 ***************
 * Copyright (C) 2003 Peter Kolb
 * kolb@ling.uni-potsdam.de 
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

/* Geometrische Funktionen zum Berechnen des Schnittpunkts 
 * zweier Geraden, des Abstands eines Punktes von einer
 * Geraden usw.
 * Nach Robert Sedgewick: Algorithmen in C. Addison-Wesley 1992, S. 399 ff.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Annahme: Koordinatensystem int x aus [0...xmax], int y aus [0...ymax] 
 *                            mit x,y > 0.
 */

/* eigene globale Definitionen */
struct point {
    int x;
    int y;
};

struct line {
    struct point p1;
    struct point p2;
};

/**********************************/
/* Hilfsfunktion fuer intersect() */
/**********************************/
int ccw(struct point p0,
	struct point p1,
	struct point p2 ){

    int dx1, dx2, dy1, dy2;
    
    dx1 = p1.x - p0.x; dy1 = p1.y - p0.y;
    dx2 = p2.x - p0.x; dy2 = p2.y - p0.y;
    if (dx1 * dy2 > dy1 * dx2) return 1;
    if (dx1 * dy2 < dy1 * dx2) return -1;
    if ((dx1 * dx2 < 0) || (dy1 * dy2 < 0)) return -1;
    if ((dx1 * dx1 + dy1 * dy1) < (dx2 * dx2 + dy2 * dy2)) return 1;
    return 0;
}

/**********************************/
/* Schnitt zweier Kanten (Linien) */
/**********************************/
int intersect(struct line l1, struct line l2){
    return ((ccw(l1.p1, l1.p2, l2.p1)
	     *ccw(l1.p1, l1.p2, l2.p2)) <= 0)
	&& ((ccw(l2.p1, l2.p2, l1.p1)
	     *ccw(l2.p1, l2.p2, l1.p2)) <= 0);
}

/**********************************************/
/* euklidischer Abstand zwischen zwei Punkten */
/**********************************************/
double punktabstand(struct point p1, 
		    struct point p2){
    return sqrt( pow(p1.x - p2.x, 2.0) + pow(p1.y - p2.y, 2.0));
}

/**********************************/
/* Abstand eines Punktes zum Rand */
/**********************************/
/* Abstand zum rechten Rand */
int r_abstand(struct point p, int xmax){
    return xmax - p.x;
}
/* Abstand zum linken Rand */
int l_abstand(struct point p){
    return p.x;
}
/* Abstand zum oberen Rand */
int t_abstand(struct point p, int ymax){
    return ymax - p.y;
}
/* Abstand zum unteren Rand */
int b_abstand(struct point p){
    return p.y;
}

/**********************************************************/
/* minimaler Abstand zwischen einem Punkt und einer Kante */
/**********************************************************/
double punkt_kante_abstand(struct point p, 
			struct line l){

    double x1 = (double) l.p1.x, y1 = (double) l.p1.y;
    double x2 = (double) l.p2.x, y2 = (double) l.p2.y;
    double x3 = (double) p.x, y3 = (double) p.y;        /* p3 = p */
    double x4, y4, z, n;

    /* Berechne die Koordinaten (x4,y4) des zu p3 naechsten Punktes p4 auf der Geraden l */
    /* Diese Formel koennte noch vereinfacht (gekuerzt) werden...                        */
    z = -y1 * y2 + (x1 * y2 * (y1 - y2))/(x1 - x2) + y2 * y3 + y1 * y1 - (x1 * y1 * (y1 - y2))/(x1 - x2) - y1 * y3 + x2 * x3 - x1 * x3;
    n = (y2 * (y1 - y2))/(x1 - x2) - (y1 * (y1 - y2))/(x1 - x2) + x2 - x1;
    x4 = z / n;
    y4 = (y1 - y2)/(x1 - x2) * x4 + y1 - (y1 - y2)/(x1 - x2) * x1;

    /* Berechne den Abstand zwischen p3 und p4 */
    return sqrt( pow(x3 - x4, 2.0) + pow(y3 - y4, 2.0));
}
