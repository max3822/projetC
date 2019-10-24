#include "Interpreteur.h"
#include <stdlib.h>
#include <iostream>
#include <valarray>
using namespace std;
int i = 0;
int j = 0;

Interpreteur::Interpreteur(ifstream & fichier) :
m_lecteur(fichier), m_table(), m_arbre(nullptr) {
}

void Interpreteur::analyse() {
    m_arbre = programme(); // on lance l'analyse de la première règle -> fonction programme
}

int Interpreteur::tester(const string & symboleAttendu) {
    // Teste si le symbole courant est égal au symboleAttendu... Si non, lève une exception
    static char messageWhat[256];
    try {
        if (m_lecteur.getSymbole() != symboleAttendu) {
            sprintf(messageWhat,
                    "Ligne %d, Colonne %d - Erreur de syntaxe - Symbole attendu : %s - Symbole trouvé : %s",
                    m_lecteur.getLigne(), m_lecteur.getColonne(), symboleAttendu.c_str(), m_lecteur.getSymbole().getChaine().c_str());
            throw SyntaxeException(messageWhat);
        }
    } catch (SyntaxeException const& e) {
        cout << e.what() << endl;
        return 0;
    }
    return 1;
}

void Interpreteur::testerEtAvancer(const string & symboleAttendu) {
    // Teste si le symbole courant est égal au symboleAttendu... Si oui, avance, Sinon, lève une exception
    if (tester(symboleAttendu))
        m_lecteur.avancer();
}

void Interpreteur::erreur(const string & message) const {
    // Lève une exception contenant le message et le symbole courant trouvé
    // Utilisé lorsqu'il y a plusieurs symboles attendus possibles...
    static char messageWhat[256];
    sprintf(messageWhat,
            "Ligne %d, Colonne %d - Erreur de syntaxe - %s - Symbole trouvé : %s",
            m_lecteur.getLigne(), m_lecteur.getColonne(), message.c_str(), m_lecteur.getSymbole().getChaine().c_str());
    throw SyntaxeException(messageWhat);
}

Noeud* Interpreteur::programme() { // on teste la struture globale du programme / on créer la séquence d'instruction entre le "procedure principale()" et le "finproc"
    // <programme> ::= procedure principale() <seqInst> finproc FIN_FICHIER
    testerEtAvancer("procedure");
    testerEtAvancer("principale");
    testerEtAvancer("(");
    testerEtAvancer(")");
    Noeud* sequence = seqInst();

    testerEtAvancer("finproc");
    tester("<FINDEFICHIER>");

    return sequence;
}

Noeud* Interpreteur::seqInst() { // a chaque fois que l'on trouve une instruction, un nouveau noeud est créé
    // <seqInst> ::= <inst> { <inst> }
    NoeudSeqInst* sequence = new NoeudSeqInst();
    do {
        inst(sequence);
    } while (m_lecteur.getSymbole() == "<VARIABLE>" || m_lecteur.getSymbole() == "si"
            || m_lecteur.getSymbole() == "tantque" || m_lecteur.getSymbole() == "pour"
            || m_lecteur.getSymbole() == "ecrire" || m_lecteur.getSymbole() == "lire"
            || m_lecteur.getSymbole() == "repeter");
    // Tant que le symbole courant est un début possible d'instruction...
    // Il faut compléter cette condition chaque fois qu'on rajoute une nouvelle instruction
    return sequence;
}

void Interpreteur::inst(NoeudSeqInst* noeud) {

    // <inst> ::= <affectation> ;| <instSiRiche> | <instTantQue> | <instRepeter> ;|
    // <instPour> |,<instEcrire> ;| <instLire> ;
    if (m_lecteur.getSymbole() == "<VARIABLE>") {// on récuoéron les erreur dans la fonction affectation, impossible de diférencier une mauvaise instruction d'un nom de variable
        Noeud *affect = affectation(); // des noeuds sont créés à chaque fois
        noeud->ajoute(affect);
    } else if (m_lecteur.getSymbole() == "si") {
        noeud->ajoute(instSi());
    } else if (m_lecteur.getSymbole() == "tantque") {
        noeud->ajoute(instTantque());
    } else if (m_lecteur.getSymbole() == "pour") {
        noeud->ajoute(pour());
    } else if (m_lecteur.getSymbole() == "ecrire") {
        noeud->ajoute(ecrire());
    } else if (m_lecteur.getSymbole() == "lire") {
        noeud->ajoute(lire());
    } else if (m_lecteur.getSymbole() == "repeter") {
        noeud->ajoute(repeter());
    } //pas besoin de défault, tout passe dans une variable / diférencier une mauvaise instruction d'un nom de variable ???
}

Noeud* Interpreteur::affectation() {
    // <affectation> ::= <variable> = <expression> 

    Noeud* noeud = nullptr;
    Symbole symbole = m_lecteur.getSymbole(); // La variable est sauvegardé
    int ligne; //sauvegarde en cas d'erreurs, pour afficher le nom d'une mauvaise instrution si ce n'est une variable (pas top) 
    int col;

    m_lecteur.avancer();
    try { // en cas d'erreurs

        if (m_lecteur.getSymbole() == "++" || m_lecteur.getSymbole() == "--") { // on test pour une itération "i++ // i--"
            Noeud* var = m_table.chercheAjoute(symbole); //on ajoute la variable à la table, on est sûr que c'est une variable (moche)
            noeud = new NoeudInstIteration(var, m_lecteur.getSymbole()); // on passe à la fonction le "++" ou "--"
            ligne = m_lecteur.getLigne();

            m_lecteur.avancer();
            if (m_lecteur.getSymbole() != ";") {
                static char messageWhat[256]; // création du message d'erreur
                sprintf(messageWhat,
                        "Ligne %d- Erreur de syntaxe : ; manquant !", //colonne sans sens
                        ligne, m_lecteur.getColonne(), symbole.getChaine().c_str()); // col et nom pour le nom et la position de l'erreur (sauvegardé avant)
                throw PointVirguleEx(messageWhat);
            }

            m_lecteur.avancer();

        } else if (m_lecteur.getSymbole() == "=") {//on test l'affectation avec le "="
            m_lecteur.avancer();
            Noeud* exp = expression(); // On mémorise l'expression trouvée ex : 3 +4 - a (fonction expression)
            Noeud* var = m_table.chercheAjoute(symbole); //on ajoute la variable à la table, on est sûr que c'est une variable (moche)     
            noeud = new NoeudAffectation(var, exp);
            testerEtAvancer(";");

        } else { //si ni l'un ni l'autre, c'est une erreur et on la traite
            static char messageWhat[256]; // création du message d'erreur
            sprintf(messageWhat,
                    "Ligne %d, Colonne %d - Erreur de syntaxe - Instruction invalide ! : %s",
                    m_lecteur.getLigne(), m_lecteur.getColonne(), symbole.getChaine().c_str()); // col et nom pour le nom et la position de l'erreur (sauvegardé avant)
            throw SyntaxeException(messageWhat);
        }


    } catch (SyntaxeException const& e) {
        this->exc = 1;
        cout << e.what() << endl;
        while (m_lecteur.getSymbole() != "<FINDEFICHIER>" && m_lecteur.getSymbole().getChaine() != ";" && m_lecteur.getSymbole() != "finproc") {
            m_lecteur.avancer(); //on cherche le ; ne sais pas si c'est bien, comme cela on retombe sur une instruction plus "valide"
        } //ex : fdsfdsfs(); lire(a); et gfdgdfg lire(a); cas 1:le lire est accepter / cas 2: le lire est sauté
        m_lecteur.avancer();

    } catch (PointVirguleEx const& e) {
        this-> exc = 1;
        cout << e.what() << endl;

    }

    return noeud;
}

Noeud* Interpreteur::expression() {

    Noeud* fact = nullptr;
    int ligne = m_lecteur.getLigne();
    // <expression> ::= <facteur> { <opBinaire> <facteur> }
    //  <opBinaire> ::= + | - | *  | / | < | > | <= | >= | == | != | et | ou
    fact = facteur();

    while (m_lecteur.getSymbole() == "+" || m_lecteur.getSymbole() == "-" ||
            m_lecteur.getSymbole() == "*" || m_lecteur.getSymbole() == "/" ||
            m_lecteur.getSymbole() == "<" || m_lecteur.getSymbole() == "<=" ||
            m_lecteur.getSymbole() == ">" || m_lecteur.getSymbole() == ">=" ||
            m_lecteur.getSymbole() == "==" || m_lecteur.getSymbole() == "!=" ||
            m_lecteur.getSymbole() == "et" || m_lecteur.getSymbole() == "ou") {
        Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
        m_lecteur.avancer();
        Noeud* factDroit = facteur(); // On mémorise l'opérande droit


        fact = new NoeudOperateurBinaire(operateur, fact, factDroit); // Et on construuit un noeud opérateur binaire
    }
    return fact; // On renvoie fact qui pointe sur la racine de l'expression
}

Noeud* Interpreteur::facteur() {

    // <facteur> ::= <entier> | <variable> | - <facteur> | non <facteur> | ( <expression> )
    Noeud* fact = nullptr;
    if (m_lecteur.getSymbole() == "<VARIABLE>" || m_lecteur.getSymbole() == "<ENTIER>") {
        fact = m_table.chercheAjoute(m_lecteur.getSymbole()); // on ajoute la variable ou l'entier à la table
        m_lecteur.avancer();
    } else if (m_lecteur.getSymbole() == "-") { // - <facteur>
        m_lecteur.avancer();
        // on représente le moins unaire (- facteur) par une soustraction binaire (0 - facteur)
        fact = new NoeudOperateurBinaire(Symbole("-"), m_table.chercheAjoute(Symbole("0")), facteur());
    } else if (m_lecteur.getSymbole() == "non") { // non <facteur>
        m_lecteur.avancer();
        // on représente le moins unaire (- facteur) par une soustractin binaire (0 - facteur)
        fact = new NoeudOperateurBinaire(Symbole("non"), facteur(), nullptr);
    } else if (m_lecteur.getSymbole() == "(") { // expression parenthésée
        m_lecteur.avancer();
        fact = expression();
        testerEtAvancer(")");
    } else { //si le facteur n'est pas connu

        try {
            static char messageWhat[256]; // création du message d'erreur
            sprintf(messageWhat,
                    "Ligne %d, Colonne %d - Erreur de syntaxe : %s n'est pas un facteur", //colonne sans sens
                    m_lecteur.getLigne(), m_lecteur.getColonne(), m_lecteur.getSymbole().getChaine().c_str()); // col et nom pour le nom et la position de l'erreur (sauvegardé avant)
            throw FacteurEx(messageWhat);

            m_lecteur.avancer();
        } catch (FacteurEx const& e) {
            this-> exc = 1;
            cout << e.what() << endl;
        }
        m_lecteur.avancer();
    }
    return fact;
}

Noeud* Interpreteur::instSi() {
    // <instSi> ::= si ( <expression> ) <seqInst> finsi
    vector<Noeud*> vectorCondition;
    vector<Noeud*> vectorSequence;
    do {
        Noeud* condition = nullptr;
        try {
            this->m_lecteur.avancer();

            if (m_lecteur.getSymbole() != "(") {
                static char messageWhat[256]; // création du message d'erreur
                sprintf(messageWhat,
                        "Ligne %d, Colonne %d - Erreur de syntaxe : ( est attendu", //colonne sans sens
                        m_lecteur.getLigne(), m_lecteur.getColonne()); // col et nom pour le nom et la position de l'erreur (sauvegardé avant)
                throw SyntaxeException(messageWhat);
            }
            m_lecteur.avancer();

            condition = expression(); // On mémorise la condition

            if (m_lecteur.getSymbole() != ")") {
                static char messageWhat[256]; // création du message d'erreur
                sprintf(messageWhat,
                        "Ligne %d, Colonne %d - Erreur de syntaxe : ( est attendu", //colonne sans sens
                        m_lecteur.getLigne(), m_lecteur.getColonne()); // col et nom pour le nom et la position de l'erreur (sauvegardé avant)
                throw SyntaxeException(messageWhat);
            }
            m_lecteur.avancer();

        } catch (SyntaxeException const& e) {


            while (m_lecteur.getSymbole() == "finsi" || m_lecteur.getSymbole() == "finproc"
                    || m_lecteur.getSymbole() == "<FINDEFICHIER>") {
                m_lecteur.avancer();
                cout << m_lecteur.getSymbole() << endl;
            }
            if (m_lecteur.getSymbole() == "finsi")
                m_lecteur.avancer();

            return nullptr;
        }


        Noeud* sequence = seqInst(); // On mémorise la séquence d'instruction  
        vectorCondition.push_back(condition);
        vectorSequence.push_back(sequence);

    } while (m_lecteur.getSymbole() == "si"
            || m_lecteur.getSymbole() == "sinonsi");

    if (m_lecteur.getSymbole() == "sinon") {
        this->m_lecteur.avancer();

        Noeud* sequence = seqInst();

        vectorCondition.push_back(nullptr);
        vectorSequence.push_back(sequence);
    }
    testerEtAvancer("finsi");

    return new NoeudInstSi(vectorCondition, vectorSequence);
}

Noeud* Interpreteur::instTantque() {
    Noeud* tq = nullptr;
    m_lecteur.avancer();

    testerEtAvancer("(");

    Noeud* condition = expression();
    testerEtAvancer(")");
    Noeud* sequence = seqInst(); // On mémorise la séquence d'instruction
    testerEtAvancer("fintantque");
    tq = new NoeudInstTantQue(condition, sequence);
    return tq;
}

Noeud* Interpreteur::pour() {

    //pour(i=0; i<10; i++;)
    Noeud* noeudPour = nullptr;
    Noeud* affect = nullptr;
    Noeud* exp = nullptr;
    Noeud* it = nullptr;

    m_lecteur.avancer();
    try {

        if (m_lecteur.getSymbole() != "(") {
            static char messageWhat[256]; // création du message d'erreur
            sprintf(messageWhat,
                    "Ligne %d, Colonne %d - Erreur de syntaxe : ( est attendu", //colonne sans sens
                    m_lecteur.getLigne(), m_lecteur.getColonne()); // col et nom pour le nom et la position de l'erreur (sauvegardé avant)
            throw SyntaxeException(messageWhat);
        }

        m_lecteur.avancer();

        affect = affectation(); // le ; est testé dans l'affectation
        exp = expression();
        tester(";");
        this->m_lecteur.avancer();
        it = affectation(); // le ; est testé dans l'affectation


        if (m_lecteur.getSymbole() != ")") {

            static char messageWhat[256]; // création du message d'erreur
            sprintf(messageWhat,
                    "Ligne %d, Colonne %d - Erreur de syntaxe : ) est attendu", //colonne sans sens
                    m_lecteur.getLigne(), m_lecteur.getColonne()); // col et nom pour le nom et la position de l'erreur (sauvegardé avant)

            throw SyntaxeException(messageWhat);
        }

        m_lecteur.avancer();

    } catch (SyntaxeException const& e) {

        cout << e.what() << endl;
        while (m_lecteur.getSymbole() != ")") {
            m_lecteur.avancer();

            if (m_lecteur.getSymbole() == "finpour" || m_lecteur.getSymbole() == "finproc"
                    || m_lecteur.getSymbole() == "<FINDEFICHIER>")
                return nullptr;
        }
        if (m_lecteur.getSymbole() == ")")
            m_lecteur.avancer();

    }


    Noeud* sequence = seqInst(); // On mémorise la séquence d'instruction
    noeudPour = new Noeudpour(affect, exp, it, sequence);

    testerEtAvancer("finpour");
    return noeudPour;
}

Noeud * Interpreteur::iterationS() {
    Noeud* it = nullptr;
    Symbole operande = m_lecteur.getSymbole();
    this->m_lecteur.avancer();
    tester("<VARIABLE>");
    Noeud* var = m_table.chercheAjoute(m_lecteur.getSymbole()); // La variable est ajoutée à la table eton la mémorise
    this->m_lecteur.avancer();
    it = new NoeudInstIteration(var, operande);
    testerEtAvancer(";");
    return it;
}

Noeud * Interpreteur::repeter() {

    testerEtAvancer("repeter");
    Noeud* sequence = seqInst();
    testerEtAvancer("jusqua");
    testerEtAvancer("(");
    Noeud* condition = expression();
    testerEtAvancer(")");
    return new Noeudrepeter(sequence, condition);

}

Noeud * Interpreteur::ecrire() {

    testerEtAvancer("ecrire");
    testerEtAvancer("(");
    if (m_lecteur.getSymbole().getChaine().at(0) != '"') { // teste de "
        erreur(" erreur chaine caractére");
    }
    int i = 0;
    string s2 = "";
    vector<Noeud*> variable;
    vector<string> chaine;
    string s1 = m_lecteur.getSymbole().getChaine(); // chaine lu
    
    //-------------------------------------------------
    m_lecteur.avancer();
    for (int j = 0; j < s1.size(); j++) { //nombre de % pour les variables
        if (s1.at(j) == '%')
            i++;
    }
    
    if (i != 0) {// variable trouvé 
        for (int j = 0; j < i; j++) {
            testerEtAvancer(",");
            tester("<VARIABLE>");
            variable.push_back(m_table.chercheAjoute(m_lecteur.getSymbole())); //on sauvegarde la variable trouvé
            m_lecteur.avancer();
        }
    }
    testerEtAvancer(")");
    testerEtAvancer(";");
    //------------------------------------------------- test de la validité : nb variable = nb %
    for (int j = 0; j < s1.size(); j++) { //parcours de la chaine initial
        s2 = s2 + s1.at(j);
        if (s1.at(j) == '%') {
            s2.pop_back(); //on retire le %
            s2 = s2 +'"';
            m_table.chercheAjoute(s2);
            chaine.push_back(s2); //on enléve le %
            s2 = '"'; // on vide s2
        }
    }
    m_table.chercheAjoute(s2);
    chaine.push_back(s2); //pour le reste de la chaine
    for(int i =0; i < chaine.size(); i++){
        cout << chaine.at(i) << endl;
    }
    
    

    return new NoeudChaineVar(variable, chaine);
}

Noeud * Interpreteur::lire() {

    //lire(a);
    this->m_lecteur.avancer();
    testerEtAvancer("(");
    vector<Noeud*> variables;
    while(m_lecteur.getSymbole().getChaine() != ")") {
        tester("<VARIABLE>");
        this->m_table.chercheAjoute(m_lecteur.getSymbole());
        Noeud* var = m_table.chercheAjoute(m_lecteur.getSymbole());
        variables.push_back(var);
        this->m_lecteur.avancer();
    }
    
    testerEtAvancer(")");
    testerEtAvancer(";");
    return new Noeudlire(variables);
}


