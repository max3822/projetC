#include <stdlib.h>
#include <string>
#include "ArbreAbstrait.h"
#include "Symbole.h"
#include "SymboleValue.h"
#include "Exceptions.h"

////////////////////////////////////////////////////////////////////////////////
// NoeudSeqInst
////////////////////////////////////////////////////////////////////////////////

NoeudSeqInst::NoeudSeqInst() : m_instructions() {
}

int NoeudSeqInst::executer() {
    for (unsigned int i = 0; i < m_instructions.size(); i++)
        m_instructions[i]->executer(); // on exécute chaque instruction de la séquence
    return 0; // La valeur renvoyée ne représente rien !
}

void NoeudSeqInst::ajoute(Noeud* instruction) {
    if (instruction != nullptr) m_instructions.push_back(instruction);
}

////////////////////////////////////////////////////////////////////////////////
// NoeudAffectation
////////////////////////////////////////////////////////////////////////////////

NoeudAffectation::NoeudAffectation(Noeud* variable, Noeud* expression)
: m_variable(variable), m_expression(expression) {
}

int NoeudAffectation::executer() {
    int valeur = m_expression->executer(); // On exécute (évalue) l'expression
    ((SymboleValue*) m_variable)->setValeur(valeur); // On affecte la variable
    return 0; // La valeur renvoyée ne représente rien !
}

////////////////////////////////////////////////////////////////////////////////
// NoeudOperateurBinaire
////////////////////////////////////////////////////////////////////////////////

NoeudOperateurBinaire::NoeudOperateurBinaire(Symbole operateur, Noeud* operandeGauche, Noeud* operandeDroit)
: m_operateur(operateur), m_operandeGauche(operandeGauche), m_operandeDroit(operandeDroit) {
}

int NoeudOperateurBinaire::executer() {
    int og, od, valeur;
    if (m_operandeGauche != nullptr) og = m_operandeGauche->executer(); // On évalue l'opérande gauche
    if (m_operandeDroit != nullptr) od = m_operandeDroit->executer(); // On évalue l'opérande droit
    // Et on combine les deux opérandes en fonctions de l'opérateur
    if (this->m_operateur == "+") valeur = (og + od);
    else if (this->m_operateur == "-") valeur = (og - od);
    else if (this->m_operateur == "*") valeur = (og * od);
    else if (this->m_operateur == "==") valeur = (og == od);
    else if (this->m_operateur == "!=") valeur = (og != od);
    else if (this->m_operateur == "<") valeur = (og < od);
    else if (this->m_operateur == ">") valeur = (og > od);
    else if (this->m_operateur == "<=") valeur = (og <= od);
    else if (this->m_operateur == ">=") valeur = (og >= od);
    else if (this->m_operateur == "et") valeur = (og && od);
    else if (this->m_operateur == "ou") valeur = (og || od);
    else if (this->m_operateur == "non") valeur = (!og);
    else if (this->m_operateur == "/") {
        if (od == 0) throw DivParZeroException();
        valeur = og / od;
    }
    return valeur; // On retourne la valeur calculée
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstSi
////////////////////////////////////////////////////////////////////////////////

NoeudInstSi::NoeudInstSi(vector<Noeud*> condition, vector<Noeud*> sequence)
: m_condition(condition), m_sequence(sequence) {
}

int NoeudInstSi::executer() {
    for (int i = 0; i < m_condition.size(); i++) {

        if (m_condition.at(i) != nullptr) {
            if (m_condition.at(i)->executer()) {
                m_sequence.at(i)->executer();
                return 0;
            }
        } else {
            m_sequence.at(i)->executer();
        }

    }
    return 0; // La valeur renvoyée ne représente rien !
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstTantQue
////////////////////////////////////////////////////////////////////////////////

NoeudInstTantQue::NoeudInstTantQue(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence) {
}

int NoeudInstTantQue::executer() {

    while (m_condition->executer()) {
        m_sequence->executer();
    }

    return 0; // La valeur renvoyée ne représente rien !
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstIteration
/////////////////////////////////////////////////////////////////////

NoeudInstIteration::NoeudInstIteration(Noeud* var, Symbole symbole)
: var(var), symbole(symbole) {
}

int NoeudInstIteration::executer() {
    int valeur = var->executer();
    if (symbole.getChaine() == "++") {
        ((SymboleValue*) var)->setValeur(valeur + 1);
    } else {
        ((SymboleValue*) var)->setValeur(valeur - 1);
    }

    return 0; // La valeur renvoyée ne représente rien !

}

NoeudChaineVar::NoeudChaineVar(vector<Noeud*> var, vector<string> chaine)
: var(var), chaine(chaine) {
}

int NoeudChaineVar::executer() {

    int variable;
    string s = ""; //chaine que l'on va créer
    string s2;
    for (int i = 0; i < var.size(); i++) {
        
        s2 = chaine.at(i);
        s2.erase(0, 1);
        s2.pop_back();

        s = s + s2; 

        variable = var.at(i)->executer();
        s = s + to_string(variable); //on convertie le int en string et on ajoute les deux
        
    }

    if (var.size() != chaine.size()) { //on récupére la dernière chaine si elle existe
        int j = var.size();
        s2 = chaine.at(j);
        s2.erase(0, 1);
        s2.pop_back();
        s = s + s2;
    }
    cout << s << endl;

    return 0; // La valeur renvoyée ne représente rien !

}

Noeudpour::Noeudpour(Noeud* affect1, Noeud* expr, Noeud* affect2, Noeud* seq)
: affect1(affect1), expr(expr), affect2(affect2), seq(seq) {
}

int Noeudpour::executer() {

    for (affect1->executer(); expr->executer(); affect2->executer()) {
        seq->executer();
    }


    return 0; // La valeur renvoyée ne représente rien !

}

Noeudlire::Noeudlire(vector<Noeud*> vars)
: vars(vars) {
}

int Noeudlire::executer() {
    Noeud* var;
    for(int j=0;j<vars.size();j++) {
        string s = "";
        while (Symbole(s) != "<ENTIER>") {
            cout << "saisir la valeur : ";
            getline(cin, s);
            if (Symbole(s) != "<ENTIER>") {
                cout << "un entier est attendu !" << endl;
            }
        }
        int i = atoi(s.c_str());
        ((SymboleValue*) vars.at(j))->setValeur(i);
    }
    return 0; // La valeur renvoyée ne représente rien !
}

Noeudrepeter::Noeudrepeter(Noeud* sequence, Noeud* condition)
: sequence(sequence), condition(condition) {
}

int Noeudrepeter::executer() {


    do {
        sequence->executer();
        //cout << "test : " << condition->executer() << endl;
    } while (!condition->executer());

    return 0; // La valeur renvoyée ne représente rien !
}