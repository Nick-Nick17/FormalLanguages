#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <stack>

class MyGrammar;
class MySituation;

using Symbol = char;
using Word = std::string;
using rule_result = std::string;

static const Symbol start = '$';
static const Symbol end = '#';

class MyGrammar {
  using set_of_rules = std::unordered_map<Symbol, std::vector<rule_result>>;

public:
  int32_t rules_nmb;     //  rules number
  int32_t term_nmb;      //  terminals number
  int32_t non_term_nmb;  //  non terminals number
  std::unordered_set<Symbol> terminals;
  std::unordered_set<Symbol> not_terminals;
  set_of_rules rules;

  void AddRule(const Symbol& key, const Word& word) {
    rules[key].push_back(word);
  }

  MyGrammar() {}

  MyGrammar(const MyGrammar& tmp);

  MyGrammar(int32_t rules_nmb, int32_t term_nmb, int32_t non_term_nmb,
            const std::string& nnterm, const std::string& term,
            const std::unordered_multimap<Symbol, Word>& ru);

  MyGrammar operator=(const MyGrammar& tmp);

  void Clear();

  bool is_terminal(Symbol token) {
    return !(terminals.find(token) == terminals.end());
  }

  bool is_not_terminal(Symbol token) {
    return (not_terminals.find(token) != not_terminals.end()) || token == start;
  }
};

std::istream& operator>>(std::istream& input, MyGrammar& grammar);
std::ostream& operator<<(std::ostream& output, const MyGrammar& grammar);

struct MySituation {
  int kursor;
  int index;
  Symbol token;
  Word rule;
  bool is_completed;

  MySituation() {}

  MySituation(int kursor, int index, Symbol token, const Word& rule) :
                  kursor(kursor), index(index), token(token), rule(rule), is_completed(false) {}
};

//  needs for using set in Early algorithm and correct round(обход)
bool operator<(const MySituation& first, const MySituation& second);

using Situation = MySituation;
using Grammar = MyGrammar;

class Earley {
private:
  struct SetOfSituations {
    std::unordered_map<Symbol, std::set<Situation>> set;

    bool IsFixSymbol(Symbol symbol) {
      return set.find(symbol) != set.end();
    }

    bool AddSituation(Symbol symbol, const Situation& stuation) {
      return set[symbol].insert(stuation).second;
    }
  };

  Grammar grammar;
  std::vector<SetOfSituations> D;
  std::vector<Situation> situations;

public:
  Earley() {}
  Earley(const Grammar& grammar) : grammar(grammar) {}

  //  clears private fields, saves grammar
  void Clear();

  //  updates private fields for a given word
  void Preparation(const Word& word);

  //  updates grammar
  void Fit(const Grammar& tmp);
  bool Complete(size_t d_ind);
  bool Predict(size_t d_ind);
  void Scan(size_t d_ind, Symbol symbol);
  bool CheckForEndSituation();
  bool Predict(Word word);
};
