/**********
 * util.c *
 **********
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

/* ------------------------------------------
 * Hilfsfunktionen fuer das Programm gravis.c
 * ------------------------------------------
 */

/******************************/
/* Anzahl der Grossbuchstaben */
/******************************/
/* einer: 0, zwei: 1 ...      */
/* Die Zeichen werden unterschiedlich breit dargestellt, daher muss man die Anzahl der 
Grossbuchstaben bestimmen und die Breite der Knoten-Ellipsen entsprechend anpassen.
*/
int anzahl_grossbuchstaben(char *wort){

    int big = 0, laenge = 0;
    char *p;
    p = wort;

    /* Als gross zaehlen: Grossbuchstaben, Bindestrich, Zahl, Sonderzeichen sowie die 
     * Buchstaben m, w und x. */
    while( *p != '\0' ){
	if ( *p == '-' || *p == '-' || *p == 'm' || *p == 'w' || *p == 'x' ||
                (*p >= 'A' && *p <= 'Z') ||
                (*p >= '0' && *p <= '9') || 
                (*p <= 'a' && *p >= 'z') ){
	    big++;
	}
	laenge++;
	p++;
    }
    big--;
    if (big == -1 ) big = 0;
    if (big == laenge-1 ) big++;
    /* bei Woertern mit =< 4 Buchstaben etwas mehr Platz lassen */
    if ( laenge <= 4 ) big++;

    return (big);
}

/*********************************/
/* Wertebereich fuer Zahl: 0-999 */
/*********************************/
char *itoa(int zahl){

    static char endung[5];
    int hunderter, zehner;

    hunderter = (int) zahl / 100;
    endung[0] = (unsigned char) 48 + hunderter;
    zehner = ((int) (zahl - hunderter * 100) / 10);
    endung[1] = (unsigned char) 48 + zehner;
    endung[2] = (unsigned char) 48 + ((int) zahl - (hunderter * 100) - (zehner * 10));
    endung[3] = '\0';
    
    return endung;
}

/*****************************************/
/* Hilfsfunktion zu setze_startzustand() */
/*****************************************/
int runden (double a){

    int i = (int) a;
    double nk = (double) a - (double) i;

    if ( nk >= 0.5 ){
	return i + 1;
    }
    else return i;
}

