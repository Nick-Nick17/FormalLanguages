#include "Earley.h"

MyGrammar::MyGrammar(const MyGrammar& tmp) : rules(tmp.rules) {
  non_term_nmb = tmp.non_term_nmb;
  rules_nmb = tmp.rules_nmb;
  term_nmb = tmp.term_nmb;
  terminals = tmp.terminals;
  not_terminals = tmp.not_terminals;
}

MyGrammar::MyGrammar(int32_t rules_nmb, int32_t term_nmb, int32_t non_term_nmb,
          const std::string& nnterm, const std::string& term,
          const std::unordered_multimap<Symbol, Word>& ru) :
          rules_nmb(rules_nmb), term_nmb(term_nmb), non_term_nmb(non_term_nmb)
          {
  for (auto symbol : term) {
    terminals.insert(symbol);
  }
  for (auto symbol : nnterm) {
    not_terminals.insert(symbol);
  }
  for (auto& [key, word] : ru) {
    if (word[0] == '|') {
      AddRule(key, "#");
    } else {
      AddRule(key, word + end);
    }
  }
}

MyGrammar MyGrammar::operator=(const MyGrammar& tmp) {
  rules = tmp.rules;
  non_term_nmb = tmp.non_term_nmb;
  rules_nmb = tmp.rules_nmb;
  term_nmb = tmp.term_nmb;
  terminals = tmp.terminals;
  not_terminals = tmp.not_terminals;
  return *this;
}

void MyGrammar::Clear() {
  terminals.clear();
  not_terminals.clear();
  rules_nmb = 0;
  term_nmb = 0;
  non_term_nmb = 0;
  rules.clear();
}

std::istream& operator>>(std::istream& input, MyGrammar& grammar) {
  grammar.Clear();
  input >> grammar.rules_nmb;
  input >> grammar.term_nmb;
  input >> grammar.non_term_nmb;
  
  std::string terminals;
  std::string not_terminals;
  input >> not_terminals;
  input >> terminals;
  for (auto it : terminals) {
    grammar.terminals.insert(it);
  }
  for (auto it : not_terminals) {
    grammar.not_terminals.insert(it);
  }
  
  for (auto i = grammar.rules_nmb; i > 0; --i) {
    Symbol key;
    input >> key;
    Word word;
    input >> word;
    input >> word;
    if (word[0] == '|') {
      grammar.AddRule(key, "#");
    } else {
      grammar.AddRule(key, word + end);
    }
  }
  return input;
}

std::ostream& operator<<(std::ostream& output, const MyGrammar& grammar) {
  output << grammar.rules_nmb << '\n';
  output << grammar.term_nmb << '\n';
  output << grammar.non_term_nmb << '\n';

  for (auto symbol : grammar.terminals) {
    std::cout << symbol;
  }
  std::cout << '\n';
  for (auto symbol : grammar.not_terminals) {
    std::cout << symbol;
  }
  std::cout << '\n';
  
  for (const auto& set_rule : grammar.rules) {
    for (const auto& rule : set_rule.second) {
      output << set_rule.first << " << ";
      output << rule << '\n';
    }
  }
  return output;
}

//  needs for using set in Early algorithm and correct round(обход)
bool operator<(const MySituation& first, const MySituation& second) {
  return (first.token < second.token) ||
         (first.token == second.token && first.rule < second.rule) ||
         (first.token == second.token && first.rule == second.rule &&
          first.kursor < second.kursor) ||
         (first.token == second.token && first.rule == second.rule &&
          first.kursor == second.kursor && first.index < second.index);
}

void Earley::Clear() {
  D.clear();
  situations.clear();
}

//  updates private fields for a given word
void Earley::Preparation(const Word& word) {
  D.resize(word.size());
  D[0].set['S'].emplace(0, 0, start, std::string("S") + end);
}

//  updates grammar
void Earley::Fit(const Grammar& tmp) {
  grammar = tmp;
  grammar.AddRule(start, std::string("S") + end);
}

bool Earley::Complete(size_t d_ind) {
  if (D[d_ind].set.find(end) == D[d_ind].set.end()) {
    return false;
  }
  bool was_change = false;
  for (auto& situation : D[d_ind].set[end]) {
    if (situation.is_completed) {
      continue;
    }
    if (D[situation.index].set.find(situation.token) == D[situation.index].set.end()) {
      const_cast<Situation&>(situation).is_completed = true;
    }
    for (auto other : D[situation.index].set[situation.token]) {
      Symbol tmp = other.rule[++other.kursor];
      was_change |= D[d_ind].AddSituation(tmp, other);
    }
    const_cast<Situation&>(situation).is_completed = true;
  }
  return was_change;
}

bool Earley::Predict(size_t d_ind) {
  bool was_change = false;
  std::unordered_set<Symbol> prediction;
  for (auto& [symbol, set] : D[d_ind].set) { 
    if (grammar.is_not_terminal(symbol) && prediction.find(symbol) == prediction.end()) {
      prediction.insert(symbol);
      for (auto& rule : grammar.rules[symbol]) {
        Situation situation(0, d_ind, symbol, rule);
        Symbol tmp = situation.rule[situation.kursor];//
        was_change |= D[d_ind].AddSituation(tmp, situation);
      }
    }
  }
  return was_change;
}

void Earley::Scan(size_t d_ind, Symbol symbol) {
  if (D[d_ind - 1].set.find(symbol) == D[d_ind - 1].set.end()) {
    return;
  }
  for (auto situation_copy : D[d_ind - 1].set[symbol]) {
    Symbol tmp = situation_copy.rule[++situation_copy.kursor];//
    D[d_ind].AddSituation(tmp, situation_copy);
  }
}

bool Earley::CheckForEndSituation() {
  return D[D.size() - 1].set[end].find(Situation(1, 0, start, std::string("S") + end)) !=
          D[D.size() - 1].set[end].end();
}

bool Earley::Predict(Word word) {
  Clear();
  word += end;
  Preparation(word);
  for (size_t i = 0; i < word.size(); ++i) {
    if (i != 0) {
      Scan(i, word[i - 1]);
    }
    bool change_ind = false;  //  change indicator
    do {
      change_ind = false;
      change_ind |= Predict(i);
      change_ind |= Complete(i);
    } while (change_ind);
  }
  return CheckForEndSituation();
}
