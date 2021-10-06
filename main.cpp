/**
 * MIT License
 * 
 * Copyright (c) 2021 SamuNatsu(https://github.com/SamuNatsu)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
**/

// Logic Expression Simplifier
// Made by SamuNatsu

// Using Quine-McCluskey Algorithm(aka Q-M Algorithm)
// Time complexity: O(N*2^N), N denotes the number of variables
// If you use all the uppercases(N = 26) as the variables, you at lease need about 175 sec to solve out

// Input like this: (AB'+A'B)'^C
// ONLY uppercases are allowed to be a variable
// You DON'T need to explicitly type in '*' for AND logic, and NO white spaces pls

// How it works:
// 1. Input expression string
// 2. Automatically add '*' for it -> O(N)
// 3. Convert it to reverse polish notation(RPN) -> O(N)
// 4. Use RPN to summon abstract syntax tree(AST) -> O(N)
// 5. Use AST to generate true value table(TVT) -> O(N*2^N)
// 6. Use TVT & Q-M Algorithm to simplify expression -> O(N^2)

// Operator priority: NOT > AND > XOR > OR

// STL includes
#include <set>
#include <stack>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <unordered_set>

// Input
std::string input;

// Analyze
std::set<char> var;
std::unordered_map<char, int> mvar;
std::vector<size_t> m;
bool validate();
void analyze();

// Main
int main() {
    // Input expression
    std::ios::sync_with_stdio(false);
    std::cout << "Input expression: ";
    std::cin >> input;

    // Validating
    if (!validate())
        return 0;

    // Analyzing
    analyze();

    return 0;
}

// Validate input
// O(N)
bool validate() {
    for (auto &i : input)
        // Check character
        if (!isupper(i) && i != '(' && i != ')' && i != '+' && i != '\'' && i != '1' && i != '0' && i != '^') {
            std::cerr << "[ERROR] Invalid character '" << i << '\'' << std::endl;
            return false;
        }
        // Count variable
        else if (isupper(i))
            var.insert(i);
    return true;
}

// Abstract syntax tree node
class OpNode {
    public:
        OpNode *l, *r;

        OpNode(): l(nullptr), r(nullptr) {}
        OpNode(const OpNode&) = delete;
        virtual ~OpNode() {
            if (l) delete l;
            if (r) delete r;
        }
        OpNode& operator=(const OpNode&) = delete;
        virtual int get() = 0;
};

// Root node
class RootNode: public OpNode {
    public:
        int get() {
            return l->get();
        }
};

// Variable node
class VarNode: public OpNode {
    private:
        char cvar;

    public:
        VarNode(char c = 1): cvar(c) {}
        int get() {
            return cvar < 2 ? cvar : mvar[cvar];
        }
};

// NOT Node
class NotNode: public OpNode {
    public:
        int get() {
            return l->get() ^ 1;
        }
};

// AND Node
class AndNode: public OpNode {
    public:
        int get() {
            return l->get() & r->get();
        }
};

// OR Node
class OrNode: public OpNode {
    public:
        int get() {
            return l->get() | r->get();
        }
};

// XOR Node
class XorNode: public OpNode {
    public:
        int get() {
            return l->get() ^ r->get();
        }
};

// Root
RootNode root;

// Priority function
int prf(char c) {
    static std::unordered_map<char, int> prl = 
        {{'(', 1}, {'+', 2}, {'^', 3}, {'*', 4}, {'\'', 5}, {')', 6}};
    return prl[c];
}

// Explicitly add AND logic
// O(N)
std::string cvtAL(const std::string& expr) {
    std::string rtn;
    rtn += expr[0];
    for (size_t i = 1; i < expr.length(); ++i) {
        if ((isupper(expr[i]) || isdigit(expr[i]) || expr[i] == '(') && expr[i - 1] != '(' && expr[i - 1] != '+' && expr[i - 1] != '^')
            rtn += '*';
        rtn += expr[i];
    }
    return rtn;
}

// Convert to reverse polish notation
// O(N)
std::string cvtRPN(const std::string& expr) {
    std::string tmp;
    std::stack<char> stk;
    for (auto &i : expr)
        if (isupper(i) || isdigit(i))
            tmp += i;
        else if (stk.empty() || i == '(')
            stk.emplace(i);
        else if (i == ')') {
            while (!stk.empty() && stk.top() != '(') {
                tmp += stk.top();
                stk.pop();
            }
            if (stk.empty())
                return "[ERROR] Invalid expression";
            stk.pop();
        }
        else if (prf(stk.top()) < prf(i))
            stk.emplace(i);
        else {
            while (!stk.empty() && prf(stk.top()) > prf(i)) {
                tmp += stk.top();
                stk.pop();
            }
            stk.emplace(i);
        }
    while (!stk.empty()) {
        tmp += stk.top();
        stk.pop();
    }
    // Compress NOT logic
    std::string rtn;
    int j = 0;
    for (size_t i = 0; i < tmp.length(); ++i)
        if (tmp[i] == '\'')
            ++j;
        else {
            if (j & 1)
                rtn += '\'';
            rtn += tmp[i];
            j = 0;
        }
    if (j & 1)
        rtn += '\'';
    return rtn;
}

// Output true value table
// O(N*2^N)
void tvt() {
    // Output title
    for (auto &i : var)
        std::cout << i << ' ';
    std::cout << "| Y" << std::endl;
    // Output table
    for (size_t i = 0, lmt = (1 << var.size()); i < lmt; ++i) {
        for (int j = var.size() - 1; j >= 0; --j)
            std::cout << ((i >> j) & 1) << ' ';
        int cnt = var.size() - 1;
        for (auto &j : var) {
            mvar[j] = ((i >> cnt) & 1);
            --cnt;
        }
        int ans = root.get();
        if (ans)
            m.emplace_back(i);
        std::cout << "| " << ans << std::endl;
    }
}

// Get 1 count
// O(N)
int count1(const std::string& str) {
    int rtn = 0;
    for (auto &i : str)
        if (i == '1')
            ++rtn;
    return rtn;
}

// Get prime list
std::vector<std::string>
gpl(const std::vector<std::string>& ls,
    std::unordered_map<std::string, std::unordered_set<size_t>>& st) {
    std::unordered_map<size_t, std::unordered_set<std::string>> cnt;
    std::vector<std::string> rtn;
    // Count element
    for (auto &i : ls)
        for (auto &j : st.at(i))
            cnt[j].emplace(i);
    // Simplify
    // O(N)
    while (cnt.size()) {
        size_t mn = ~0ull;
        int mns;
        // Find min element count
        for (auto &i : cnt)
            if (i.second.size() < mn) {
                mn = i.second.size();
                mns = i.first;
            }
        // Find max element set
        mn = 0;
        std::string ms;
        for (auto &i : cnt[mns])
            if (st[i].size() > mn) {
                mn = st[i].size();
                ms = i;
            }
        rtn.emplace_back(ms);
        // Delete
        for (auto &i : st[ms]) {
            for (auto &j : st)
                if (j.first != ms)
                    j.second.erase(i);
            cnt.erase(i);
        }
        st[ms].clear();
    }
    return rtn;
}

// Quine-McCluskey Algorithm
// O(N^2)
std::vector<std::string> QMA() {
    std::vector<std::string> ls, tls;
    std::unordered_map<std::string, std::unordered_set<size_t>> st;
    // Convert to string
    for (auto &i : m) {
        std::string tmp;
        for (int j = var.size() - 1; j >= 0; --j)
            tmp += ((i >> j) & 1) + '0';
        ls.emplace_back(tmp);
        st[tmp].emplace(i);
    }
    // Merge
    // O(N^2)
    bool f = false;
    std::unordered_map<int, std::vector<std::string>> grp;
    std::unordered_map<std::string, bool> chk;
    do {
        chk.clear();
        tls.clear();
        grp.clear();
        f = false;
        int mx = -1;
        // Grouping
        for (auto &i : ls) {
            int tmp = count1(i);
            grp[tmp].emplace_back(i);
            if (tmp > mx)
                mx = tmp;
        }
        // Compare with adjacent group
        for (size_t i = 1; i <= mx; ++i)
            if (grp[i].size() && grp[i - 1].size())
                for (auto &j : grp[i])
                    for (auto &k : grp[i - 1]) {
                        std::string tmp;
                        int cnt = 0;
                        for (size_t l = 0; cnt < 2 && l < j.size(); ++l)
                            if (j[l] != k[l]) {
                                tmp += '-';
                                ++cnt;
                            }
                            else 
                                tmp += j[l];
                        if (cnt != 1)
                            continue;
                        if (st.find(tmp) == st.end()) {
                            st[tmp] = st[j];
                            for (auto &_ : st[k])
                                st[tmp].emplace(_);
                            tls.emplace_back(tmp);
                        }
                        chk[j] = chk[k] = true;
                        f = true;
                    }
        for (auto &i : ls)
            if (!chk[i])
                tls.emplace_back(i);
        ls.swap(tls);
    } while (f);
    return gpl(ls, st);
}

// Assert
void ast(const std::string& err, std::stack<OpNode*>& stk) {
    // Clear stack
    while (!stk.empty()) {
        delete stk.top();
        stk.pop();
    }
    // Output error
    std::cerr << err << std::endl;
}

// Analyze
void analyze() {
    // Get reverse polish notation
    std::string rpn = cvtRPN(cvtAL(input));
    if (rpn[0] == '[') {
        std::cerr << rpn << std::endl;
        return;
    }
    // Get abstract syntax tree
    std::stack<OpNode*> stk;
    auto errout = std::bind(ast, std::placeholders::_1, stk);
    for (auto &i : rpn)
        if (isupper(i))
            stk.emplace(new VarNode(i));
        else if (isdigit(i))
            stk.emplace(new VarNode(i - '0'));
        else if (i == '\'') {
            if (stk.size() < 1) {
                errout("[ERROR] Invalid NOT logic");
                return;
            }
            NotNode *tmp = new NotNode();
            tmp->l = stk.top();
            stk.pop();
            stk.emplace(tmp);
        }
        else if (i == '*') {
            if (stk.size() < 2) {
                errout("[ERROR] Invalid AND logic");
                return;
            }
            AndNode *tmp = new AndNode();
            tmp->l = stk.top();
            stk.pop();
            tmp->r = stk.top();
            stk.pop();
            stk.emplace(tmp);
        }
        else if (i == '^') {
            if (stk.size() < 2) {
                errout("[ERROR] Invalid XOR logic");
                return;
            }
            XorNode *tmp = new XorNode();
            tmp->l = stk.top();
            stk.pop();
            tmp->r = stk.top();
            stk.pop();
            stk.emplace(tmp);
        }
        else if (i == '+') {
            if (stk.size() < 2) {
                errout("[ERROR] Invalid OR logic");
                return;
            }
            OrNode *tmp = new OrNode();
            tmp->l = stk.top();
            stk.pop();
            tmp->r = stk.top();
            stk.pop();
            stk.emplace(tmp);
        }
        else {
            errout("[ERROR] Invalid logic");
            return;
        }
    if (stk.size() > 1) {
        errout("[ERROR] Invalid logic");
        return;
    }
    root.l = stk.top();
    std::cout << std::endl;
    // If is constant expression
    if (var.size() == 0) {
        std::cout << "Constant expression:\nY = " << root.get() << std::endl;
        return;
    }
    // Output true value table
    tvt();
    // Output minimum expression
    std::cout << "\nY = m("; 
    for (size_t i = 0; i < m.size(); ++i) {
        if (i)
            std::cout << ',';
        std::cout << ' ' << m[i];
    }
    std::cout << ")\n" << std::endl;
    // Output simplified expression
    if (m.size() == 0) {
        std::cout << "Y = 0" << std::endl;
        return;
    }
    if (m.size() == (1 << var.size())) {
        std::cout << "Y = 1" << std::endl;
        return;
    }
    auto sl = QMA();
    std::cout << "Y = ";
    std::vector<std::string> lss;
    for (size_t i = 0; i < sl.size(); ++i) {
        std::string tmp;
        int cnt = 0;
        for (auto &j : var) {
            if (sl[i][cnt] == '0') {
                tmp += j;
                tmp += '\'';
            }
            else if (sl[i][cnt] == '1')
                tmp += j;
            ++cnt;
        }
        lss.emplace_back(tmp);
    }
    std::sort(lss.begin(), lss.end());
    for (size_t i = 0; i < lss.size(); ++i) {
        if (i)
            std::cout << '+';
        std::cout << lss[i];
    }
    std::cout << std::endl;
}
