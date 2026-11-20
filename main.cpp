#include <cstddef>
#include <iostream>
#include <memory>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>

// dichiaro variabili che mi serviranno
struct symbol;
symbol parse(const std::string &str);

// controllo che una stringa sia un numero "puro" e non simbolica
bool isNum(const std::string &str) {
  for (const unsigned char i : str) {
    if (i < '0' || i > '9') {
      return false;
    }
  }
  return true;
}

// trovare la virgola al livello 0 di parentesi
// servirà per interpretare le funzioni a più parametri
size_t findComma0(const std::string &str) {
  size_t pos = std::string::npos;
  int count = 0;
  for (size_t i = 0; i < str.length(); i++) {
    switch (str[i]) {
    case '(':
      count++;
      break;
    case ')':
      count--;
      break;
    case ',':
      if (count == 0)
        pos = i;
      break;
    }
  }
  return pos;
}

// definizione del tipo simbolico
struct symbol {
  // consiste di due stringhe e un operatore
  // es: a+b -> symbol(a, +, b)
  std::string sym1;
  std::string sym2;
  char op = '\0';
  // costruttore completo
  symbol(const std::string &sym1, const char op, const std::string &sym2)
      : sym1(sym1), sym2(sym2), op(op) {};
  // costruttore default
  symbol() {}
  // costruttore parziale per simboli terminali
  symbol(const std::string &input) : symbol(parse(input)) {
    std::cerr << "symbol constructed: " << input << '\n';
  }
  // funzione booleana che capisce se un simbolo è terminale
  // obsoleta dato che viene gestita da parse questa cosa
  bool checkTerm() const { return (op == '\0'); };
  // non so bene nemmeno io, non mi ricordo cosa ho fatto
  // non credo che questa funzione verrà mai usata
  symbol exp(int n) {
    return symbol("exp(" + toString() + "," + std::to_string(n) + ')');
  }
  // qui c'è l'overriding dell'operatore <<
  // se uso std::cout << symbol(x) mi butta a console una cosa formattata
  friend std::ostream &operator<<(std::ostream &os, symbol sym) {
    os << sym.sym1 << sym.op << sym.sym2;
    return os;
  }
  // regole di semplificazione di un simbolo (finalmente qualcosa di matematico)
  symbol simplify() const {
    // controllo della terminalità del simbolo
    if (op != '\0') { // se il simbolo non è terminale
      if (isNum(sym1) && isNum(sym2)) {
        // se sym1 e sym2 sono numeri si usano le regole algebriche
        switch (op) {
        case '+':
          return std::to_string((std::stoi(sym1) + std::stoi(sym2)));
        case '-':
          return std::to_string((std::stoi(sym1) - std::stoi(sym2)));
        case '*':
          return std::to_string(std::stoi(sym1) * std::stoi(sym2));
        }
      } else {
        // altrimenti se non sono numerici ho messo qualche identità simbolica
        switch (op) {
        // con i '+' ho messo che 0+a e a+0 ritornino a
        case '+':
          if (sym1 == "0") {
            if (sym2 == "0") {
              return symbol("0");
            } else {
              return symbol(sym2);
            }
          } else if (sym2 == "0") {
            return symbol(sym1);
          }
          break;
        // con i '-' ho messo quasi la stessa cosa dei '+'
        case '-':
          if (sym2 == "0") {
            if (sym1 == "0") {
              return symbol("0");
            } else {
              return symbol(sym1);
            }
          } else {
            return *this;
          }
          break;
        // con i '*' ho messo a*1 o 1*a ritorna a, e  0*a o a*0 ritorna 0
        case '*':
          if (sym1 == "1") {
            return symbol(sym2);
          } else if (sym2 == "1") {
            return symbol(sym1);
          } else if (sym1 == "0" || sym2 == "0") {
            return symbol("0");
          }
          break;
        // con i '/' ho messo a/1 = a, e a/a = 1 se a != 0
        case '/':
          if (sym2 == "1") {
            return symbol(sym1);
          } else if (sym1 == sym2 && sym1 != "0") {
            return symbol("1");
          }
          break;
        }
      }
    } else {
      // qui entriamo nel campo dei simboli terminali
      // pos trova la prima parentesi aperta
      size_t pos = sym1.find('(');
      std::string toS;
      std::string input;
      if (pos != 0 && pos != std::string::npos) { // se pos esiste e non è il
                                                  // primo carattere allora...
        // toSimplify diventa da 0 a pos es: sin(...) -> toS = "sin"
        toS = sym1.substr(0, pos);
        // input è ciò che sta nelle parentesi
        input = sym1.substr(pos + 1, sym1.length() - pos - 2);
        // riscrivo pos come posizione della virgola relativa ad input
        pos = findComma0(input);
      }
      // vari casi di funzioni diverse:
      // se la posizione è un esponenziale, cioè f(x)^g(x) allora:
      if (toS == "exp" && pos != std::string::npos) {
        // f, cioè first e s, cioè second vengono valorizzati come stringhe:
        // f = il parametro sx alla virgola
        // s = il parametro dx alla virgola
        const std::string f = input.substr(0, pos), s = input.substr(pos + 1);
        // casi gestiti: x^1 e x^0
        if (s == "1")
          return symbol(f);
        else if (s == "0" && f != "0")
          return symbol("1");
      }
    }
    // se non trova nulla da semplificare ritorna il simbolo stesso non cambiato
    return *this;
  };

  // funzione che trasforma un simbolo nella stringa corrispondente ad esso
  std::string toString() const {
    if (checkTerm())
      return sym1;
    return std::string('(' + sym1 + op + sym2 + ')');
  }

  // overriding di operatori +-*/ per i simboli (addiz e molt di polinomi)
  symbol operator+(const symbol &sym) const {
    return symbol(toString(), '+', sym.toString());
  }
  symbol operator-(const symbol &sym) const {
    return symbol(toString(), '-', sym.toString());
  }
  symbol operator*(const symbol &sym) const {
    return symbol(toString(), '*', sym.toString());
  }
  symbol operator/(const symbol &sym) const {
    return symbol(toString(), '/', sym.toString());
  }
};

// funizione che elimina gli spazi da una stringa, utile per il parsing
std::string delWhiteSpace(const std::string &string) {
  std::string output;
  for (unsigned char it : string) {
    if (it != ' ')
      output.push_back(it);
  }
  return output;
};

// funzione di parsing, trova la operazione principale del polinomio
// e divide il polinomio in due pezzi
symbol parse(const std::string &toParse) {
  // logging
  std::cerr << "parse: " << toParse << '\n';
  // formattazione della stringa di input
  std::string input = delWhiteSpace(toParse);
  // se la stringa è vuota ritorna simbolo vuoto
  if (input.empty())
    return symbol();
  // inizializzazione di alcune variabili
  // pos -> indicherà la posizione del segno
  // add -> indicherà se è trovata una addizione algebrica
  // inPars -> inizialmente controlla se il primo e l'ultimo carattere sono ()
  // serve a capire se la funzione è circondata da parentesi, tipo (x+1)
  // e serve a trasformarla in x+1, successivamente verrà confermato o smentito
  // countPars -> conta le parentesi, serve per capire inPars e trovare il segno
  size_t pos = std::string::npos;
  bool add = 0;
  bool mul = 0;
  bool inPars = (input[0] == '(' && input[input.size() - 1] == ')');
  int countPars = 0;
  // ciclo che itera tutta la stringa formattata
  for (size_t i = 0; i < input.size(); i++) {
    // switch controlla diverse cose
    switch (input[i]) {
      // controllo parentesi, se ( allora contaparentesi sale di uno
      // se ) allora contaparentesi scende di uno
    case '(':
      countPars++;
      break;
    case ')':
      countPars--;
      break;
    // se addizione algebrica e livello di parentesi = 0 allora
    // valorizzare la posizione pos
    case '+':
    case '-':
      if (countPars == 0) {
        add = true;
        pos = i;
      }
      break;
    // se non si trova addizione algebrica ci si acocntenta di moltiplicazione
    case '/':
    case '*':
      if (!add && countPars == 0) {
        mul = true;
        pos = i;
      }
      break;
    default:
      break;
    }
    // controllo di inPars
    // se il contaparentesi scende a 0 prima di finire il ciclo allora non siamo
    // in parentesi
    if (countPars == 0 && i < input.size() - 1)
      inPars = false;
  }
  // finito ciclo analisi stringa
  // se siamo in parentesi
  if (inPars) {
    // rieseguire la funzione di parsing con le parentesi tolte
    return parse(input.substr(1, input.size() - 2));
  }
  // qui sappiamo che inPars = false
  // valorizzo alcuni valori
  std::string L = input, // L lo inizializzo come l'input per convenzione
      R;                 // L e R sono Left e Right, parti dx e sx del simbolo
  char op = '\0';        // operatore lo inizializzo come '\0' per convenzione
  // controllo che sia stato trovato un segno +-*/
  if (pos != std::string::npos) {
    // se la posizione è 0 si ha un segno unario, tipo +3 o -2
    if (pos == 0)
      // assegno "0" a L e poi lo sommo al R
      L = "0";
    else
      // se non c'è segno unario do ad L la parte sx al segno della stringa
      L = input.substr(0, pos);
    // do ad R la parte dx della stringa in ogni caso
    R = input.substr(pos + 1);
    // do as op il valore del segno trovato a liv 0 di parentesi
    op = input[pos];
  }
  // faccio il return di un simbolo costruito e valorizzato con queste regole
  return symbol(L, op, R);
}

// versione verbosa del parsing
symbol parseVerbose(const std::string &toParse) {
  symbol output = parse(toParse);
  std::cout << output << '\n';
  return output;
}

// altre versioni del parse perchè volevo
symbol parse(const symbol &sym) {
  if (!sym.checkTerm()) {
    return parse(sym.sym1);
  }
  return symbol(sym);
}

// struct polinomio, un albero binario di simboli
struct polinomio {
  symbol data; // contiene un simbolo
  std::unique_ptr<polinomio> L = nullptr;
  std::unique_ptr<polinomio> R = nullptr;
  // contiene due puntatori ad altri alberi
  // che contengono una simbolo

  // costruttore dato un simbolo
  polinomio(const symbol &T) : data(T) {
    // se il simbolo non è terminale valorizza i nodi successivi dell'albero
    if (!data.checkTerm()) {
      this->L = std::make_unique<polinomio>(parse(T.sym1));
      this->R = std::make_unique<polinomio>(parse(T.sym2));
    }
  }

  // costruttore basato su stringa, molto semplice
  polinomio(const std::string &string) : polinomio(parse(string)) {}

  // funzione di semplifica di polinomio basata su quella dei simboli
  void simplify() {
    // se il nodo non è terminale
    if (this->L != nullptr) {
      // richiama la funzione di semplificazione sui nodi successivi
      L->simplify();
      R->simplify();
      // modifica il nodo corrente con regole algebriche
      // sostituisce il nodo corrente con la sua versione semplificata
      switch (data.op) {
      case '+':
        data = (L->data + R->data).simplify();
        break;
      case '-':
        data = (L->data - R->data).simplify();
        break;
      case '*':
        data = (L->data * R->data).simplify();
        break;
      case '/':
        data = (L->data / R->data).simplify();
        break;
      default:
        // error handling di funzioni non trovate
        throw std::runtime_error("Operatore non supportato");
      }
      // elimina i nodi successivi
      L.reset();
      R.reset();
    } else {
      // se il nodo è terminale
      // semplifica direttamente il simbolo del nodo stesso
      data = data.simplify();
    }
  }

  // ciclo di semplificazioni
  // continua a semplificare finchè il risultato non converge
  static std::string simplifyFully(polinomio &pol, unsigned int n = 0) {
    std::string input = pol.data.toString();
    pol.simplify();
    if (n == 100) {
      return pol.data.toString();
    } else if (pol.data.toString() != input) {
      return simplifyFully(pol, ++n);
    } else {
      return pol.data.toString();
    }
  }
};

// funzione che ritorna un oggetto optional:
// riconosce le cose prima delle parentesi,
// se non c'è nulla ritorna un oggetto "vuoto", speciale
// sennò ritorna una stringa contenente ciò che c'è prima
std::optional<std::string> getBeforePars(const std::string &input) {
  size_t pos = input.find('(');
  if (pos != std::string::npos)
    return input.substr(0, pos);
  return std::nullopt;
}

// codice inutile e superato per calcolare derivate

/*
symbol D(const polinomio &pol);

symbol D(const std::string &sym) {
  size_t comma = std::string::npos;
  const auto toD = getBeforePars(sym);
  if (toD.has_value()) {
    std::vector<size_t[2]> pars;
    const std::string input = sym.substr(sym.find('('));
    bool inPars = (input[0] == '(' && input[input.size() - 1] == ')');
    for (size_t i = 0; i < sym.size() - 1; i++) {
      unsigned int count = 0;
      if (sym.at(i) == '(') {
        count++;
      }
      if (sym.at(i) == ')') {
        count--;
      }
      if (count == 0 && sym.at(i) == ',') {
        comma = i;
      }
      if (count == 0 && i < input.size() - 1)
        inPars = false;
    }
    if (comma != std::string::npos) {
      size_t pos[2] = {sym.find('('), sym.find(')')};

      if (toD == "sin") {
        return symbol("cos(" + sym.substr(pos[0], pos[1]) + ')', '*',
                      D(polinomio(sym.substr(pos[0], pos[1]))).toString());
      };
      if (toD == "cos") {
        return symbol("-sin(" + sym.substr(pos[0], pos[1]) + ')', '*',
                      D(polinomio(sym.substr(pos[0], pos[1]))).toString());
      };
      if (toD == "exp") {
        return symbol("exp(" + sym.substr(pos[0], pos[1]) + ')', '*',
                      D(polinomio(sym.substr(pos[0], pos[1]))).toString());
      }
    } else {
      size_t pos[2] = {sym.find('('), sym.find_last_of(')')};
      if (toD == "exp") {
      }
    }
  };
  size_t pos = sym.find('x');
  if (pos != std::string::npos) {
    std::string before = sym.substr(0, pos);
    if (before == "") {
      return symbol("1");
    } else {
      return symbol(before);
    };
  };
  return symbol("0");
};

symbol D(const polinomio &pol) {
  if (pol.data.op != '\0') {
    switch (pol.data.op) {
    case '+':
      return D(*pol.L) + D(*pol.R);
    case '-':
      return D(*pol.L) - D(*pol.R);
    case '*':
      return (D(*pol.L) * (pol.R->data)) + (D(*pol.R) * (pol.L->data));

    case '/':
      return ((D(*pol.L) * (pol.R->data)) - (D(*pol.R) * pol.L->data)) /
             (pol.R->data.exp(2));
    default:;
    }
  }
  return D(pol.data.sym1);
};
*/

// funzione che controlla che una stringa sia circondata da parentesi
// anche questa superata da parse
bool inPars(const std::string &str) {
  if (str.empty() || str[0] != '(' || str.back() != ')')
    return false;
  int count = 0;
  for (size_t i = 0; i < str.length(); i++) {
    switch (str[i]) {
    case '(':
      count++;
      break;
    case ')':
      count--;
      break;
    }
    if (count < 0)
      return false;
    if (count == 0 && i != str.length() - 1)
      return false;
  }
  return count == 0;
}

// funzione di derivazione, supera quelle commentate di sopra
symbol D(const symbol &sym) {
  std::cerr << "D symbol: " << sym.sym1 << '\n';
  // controllo che il simbolo non sia terminale es:
  // costante, x, sin(f(x)) f(g(x)) ...
  if (sym.op != '\0') {
    // switch che gestisce i 4 operatori fondamentali
    switch (sym.op) {
    case '+':
      return symbol(D(sym.sym1) + D(sym.sym2)).simplify();
    case '-':
      return symbol(D(sym.sym1) - D(sym.sym2)).simplify();
    case '*':
      return symbol((D(sym.sym1) * sym.sym2) + (D(sym.sym2) * sym.sym1))
          .simplify();
    case '/':
      return symbol(((D(sym.sym1) * (sym.sym2)) - (D(sym.sym2) * sym.sym1)) /
                    (symbol(sym.sym2).exp(2)))
          .simplify();
    // se non viene troavato un operatore valido allora BAM, errore
    default:
      throw std::runtime_error(std::string("operatore non supportato: ") +
                               sym.op);
    }

  } // gestione di simboli terminali (altrimenti)
  else {
    // crea la stringa della funzione da derivare es:
    // "sin(f(x))" -> toD = "sin"
    // se non c'è nulla prima delle parentesi ritorna un elemento speciale
    const auto toD = getBeforePars(sym.sym1);
    // controlla se toD ha un valore grazie al valore "speciale"
    if (toD.has_value()) {

      // trova la posizione della parentesi aperta e di quella chiusa es:
      // //sin->(f(x)) <-
      size_t pars[2] = {sym.sym1.find('('), sym.sym1.find_last_of(')')};
      // genera una stringa che abbia come valore l'argomento della funzione
      const std::string input =
          sym.sym1.substr(pars[0] + 1, pars[1] - pars[0] - 1);
      // cerca una virgola nell'input per vedere se è a uno o due parametri
      size_t comma = findComma0(input);
      // se una virgola non è stata trovata calcolo le funzioni ad un parametro

      if (comma == std::string::npos) {
        /* creo il simbolo di D(input) perchè calcolo tutto come f(g(x))
        lo creo solo se non c'è una virgola, così sono sicuro di avere
        funzioni ad un parametro */
        symbol Dinput = D(input).simplify();

        // formule varie
        if (*toD == "sin") {
          symbol cosX("cos(" + input + ')');
          return symbol(cosX * Dinput).simplify();
        }
        if (*toD == "cos") {
          symbol nSinX("-sin(" + input + ')');
          return symbol(nSinX * Dinput).simplify();
        }

        if (*toD == "exp") {
          symbol exp(*toD);
          return symbol(exp * Dinput).simplify();
        }
        if (*toD == "ln") {
          return symbol(Dinput / input).simplify();
        } // se si trova una funzione sconosciuta BAM errore
        else {
          throw std::runtime_error("Funzione non supportata: " + *toD);
        }
      } // se una virgola è stata trovata calcolo le funzioni a 2 parametri
      else {
        // estraggo il primo e il seocondo parametro della funzione
        const symbol f = input.substr(0, comma), // primo parametro
            s = input.substr(comma + 1);         // secondo parametro
        if (*toD == "exp") {
          symbol power("exp(" + f.toString() + ',' + s.toString() + ")");
          symbol lnF("ln(" + f.toString() + ")");
          return symbol(power * (D(s) * lnF + s * (D(f) / f))).simplify();
        } else {
          throw std::runtime_error("Funzione non supportata " + *toD);
        }
      }
    }
    // se toD non ha valore abbiamo 2 casi:
    // 1) c * x oppure x (che sarebbe 1*x) -> 123x
    // (dato che l'utente non ha immesso il segno è moltiplicazione implicita)
    // 2) c e basta -> 34524
    // ovviamente se incontriamo il primo ritorniamo c sennò 0
    else {
      // raccogliamo la posizione di x
      size_t x = sym.sym1.find('x');
      // se la posizione di x esiste prendiamo la parte di stringa prima e
      // dopo di x in before e after
      if (x != std::string::npos) {
        std::string before = sym.sym1.substr(0, x),
                    after = sym.sym1.substr(x + 1), tot;
        // tot diventa std::string() se before e after sono vuote
        tot = after + before;
        // se tot è std::string() ritorna 1, c'è solo la x
        if (tot == std::string()) {
          return symbol("1");
        } // altrimenti ritorna before * after con le appropriate parentesi
        else {
          if (before == std::string()) {
            return symbol(after).simplify();
          } else if (after == std::string()) {
            return symbol(before).simplify();
          } else {
            return symbol('(' + before + ")*(" + after + ')').simplify();
          }
        }
      } // se non si trova x ritorna 0, è una costante
      else {
        return symbol("0");
      }
    }
  }
}

// altre funzioni di derivazione, astrazione inutile
symbol D(const std::string &str) {
  if (str == "q" || str == "exit" || str == "Q")
    throw std::runtime_error("exiting...");
  return D(symbol(str));
}

symbol D(const polinomio &pol) { return D(pol.data); }

// il main, codice semplice e abbastanza leggibile
int main(int argc, char *argv[]) {
  // creo una stringa vuota
  std::string input;
  // loop infinito di feedback utente
  while (true) {
    // metto a schermo la richiesta di inserire una funzione
    std::cout << "Inserisci funzione: ";
    // raccolgo ciò che l'utente scrive e se scrive CTRL + D esco
    if (!std::getline(std::cin, input))
      break;
    // try e catch servono a gestire gli errori
    try {
      // valorizzo un polinomio data la derivata della stringa inserita
      polinomio Dx = polinomio(D(input));
      // semplifico il polinomio
      polinomio::simplifyFully(Dx);
      // mando a schermo il polinomio derivato e semplificato
      std::cout << Dx.data << '\n';
    } catch (const std::exception &e) {
      // se sono trovati errori informa l'utente ed esci
      // questo non è un errore, è una eccezione che ho aggiunto
      // serve per poter uscire con exit, q, Q
      if (std::string(e.what()) == "exiting...")
        break;
      else
        std::cerr << "Errore: " << e.what() << '\n';
    }
  }
  // se il programma va a buon fine ritornare 0
  return 0;
}
