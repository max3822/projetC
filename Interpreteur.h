#ifndef INTERPRETEUR_H
#define INTERPRETEUR_H

#include "Symbole.h"
#include "Lecteur.h"
#include "Exceptions.h"
#include "TableSymboles.h"
#include "ArbreAbstrait.h"
#include <vector>

class Interpreteur {
public:
    Interpreteur(ifstream & fichier); // Construit un interpréteur pour interpreter
    //  le programme dans  fichier 

    void analyse(); // Si le contenu du fichier est conforme à la grammaire,
    //   cette méthode se termine normalement et affiche un message "Syntaxe correcte".
    //   la table des symboles (ts) et l'arbre abstrait (arbre) auront été construits
    // Sinon, une exception sera levée

    inline const TableSymboles & getTable() const {
        return m_table;
    } // accesseur	

    inline Noeud* getArbre() const {
        return m_arbre;
    }

    inline int getExc() const {
        return exc;
    }// accesseur

private:
    Lecteur m_lecteur; // Le lecteur de symboles utilisé pour analyser le fichier
    TableSymboles m_table; // La table des symboles valués
    Noeud* m_arbre; // L'arbre abstrait
    int exc = 0;

    // Implémentation de la grammaire
    Noeud* programme(); //   <programme> ::= procedure principale() <seqInst> finproc FIN_FICHIER
    Noeud* seqInst(); //     <seqInst> ::= <inst> { <inst> }
    void inst(NoeudSeqInst* noeud); //        <inst> ::= <affectation> ; | <instSi>
    //        <inst> ::= <affectation> ;| <instSiRiche> | <instTantQue> | <instRepeter> ;| <instPour> |,<instEcrire> ;| <instLire> ;

    Noeud* affectation(); // <affectation> ::= <variable> = <expression> 
    Noeud* expression(); //  <expression> ::= <facteur> { <opBinaire> <facteur> }
    Noeud* facteur(); //     <facteur> ::= <entier>  |  <variable>  |  - <facteur>  | non <facteur> | ( <expression> )
    //   <opBinaire> ::= + | - | *  | / | < | > | <= | >= | == | != | et | ou
    Noeud* instSi(); //      <instSi> ::= si ( <expression> ) <seqInst> finsi
    Noeud* instTantque();
    Noeud* pour();
    Noeud* iterationS();

    Noeud* repeter();
    Noeud* ecrire();
    Noeud* lire();



    // outils pour simplifier l'analyse syntaxique
    int tester(const string & symboleAttendu); //0 si faux, 1 si vrai
    void testerEtAvancer(const string & symboleAttendu); // Si symbole courant != symboleAttendu, on lève une exception, sinon on avance
    void erreur(const string & mess) const; // Lève une exception "contenant" le message mess
    void testPoint() const;
    
};

#endif /* INTERPRETEUR_H */
