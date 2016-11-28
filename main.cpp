#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>

using namespace std;

///////////////////////////////////////////////////////////////////////////////////
//                                  MODEL
///////////////////////////////////////////////////////////////////////////////////
class Point {
public:
    float x;
    float y;

    explicit Point(float x, float y)
    : x(x)
    , y(y)
    {}

    float distance2(Point const& p) const {
        return (x - p.x)*(x - p.x) + (y - p.y)*(y - p.y);
    }

    float distance(Point const& p) const {
        return sqrt(distance2(p));
    }
};

class Unit : public Point {
public:
    int id;
    float vx;
    float vy;
    float r;

    explicit Unit(int id, float x, float y, float vx, float vy, float r)
    : Point(x, y)
    , id(id)
    , vx(vx)
    , vy(vy)
    , r(r)
    {}

    bool contains(Point const& p) const {
        return distance(p) <= r;
    }

    void move() {
        x += vx;
        y += vy;
    }
};

class Wizard : public Unit {
public:
    int state;

    explicit Wizard(int id, float x, float y, float vx, float vy, int state)
    : Unit(id, x, y, vx, vy, 400.f)
    , state(state)
    {}
};
using Wizards = vector<Wizard>;

class Snaffle : public Unit {
public:
    explicit Snaffle(int id, float x, float y, float vx, float vy)
    : Unit(id, x, y, vx, vy, 150.f)
    {}
};
using Snaffles = vector<Snaffle>;

class Bludger : public Unit {
public:
    float dist;
    Wizard const* target;

    explicit Bludger(int id, float x, float y, float vx, float vy)
    : Unit(id, x, y, vx, vy, 200.f)
    , dist(999999999.f)
    , target(nullptr)
    {}

    void updateTarget(Wizards wizards) {
        cerr << "updateTarget bludger id: " << id << " dist: " << dist << endl;
        for (auto const& wizard: wizards) {
            float new_dist = wizard.distance(*this);
            if (new_dist < dist) {
                cerr << "updateTarget bludger id: " << id << " perform update, new_dist: " << new_dist << " old_dist: " << dist << " wizard: " << wizard.id << endl;
                dist = new_dist;
                target = &wizard;
            }
        }
    }
};
using Bludgers = vector<Bludger>;

///////////////////////////////////////////////////////////////////////////////////
//                                 GLOBALS
///////////////////////////////////////////////////////////////////////////////////
int teamId;
Wizards my_wizards;
Wizards his_wizards;
Snaffles snaffles;
Bludgers bludgers;
int mana;

///////////////////////////////////////////////////////////////////////////////////
//                                 UTILS
///////////////////////////////////////////////////////////////////////////////////
void loadInput() {
    int entities; // number of entities still in game
    cin >> entities; cin.ignore();
    for (int i = 0; i < entities; i++) {
        int id; // entity identifier
        string entityType; // "WIZARD", "OPPONENT_WIZARD", "SNAFFLE" or "BLUDGER"
        int x; // position
        int y; // position
        int vx; // velocity
        int vy; // velocity
        int state; // 1 if the wizard is holding a Snaffle, 0 otherwise
        cin >> id >> entityType >> x >> y >> vx >> vy >> state; cin.ignore();

        if (entityType == "WIZARD") {
            my_wizards.emplace_back(id, x, y, vx, vy, state);
        }
        else if (entityType == "OPPONENT_WIZARD") {
            his_wizards.emplace_back(id, x, y, vx, vy, state);
        }
        else if (entityType == "SNAFFLE") {
            snaffles.emplace_back(id, x, y, vx, vy);
        }
        else if (entityType == "BLUDGER") {
            bludgers.emplace_back(id, x, y, vx, vy);
        }
        else {
            assert(false && "Wrong entity type");
        }
    }
}

void clear() {
    my_wizards.clear();
    his_wizards.clear();
    snaffles.clear();
    bludgers.clear();
}

int main()
{
    mana = 0;
    cin >> teamId; cin.ignore();
    while (1) {
        loadInput();

        for (auto & bludger : bludgers) {
            bludger.updateTarget(his_wizards);
            bludger.updateTarget(my_wizards);
        }

        cerr << "begin: " << snaffles.size() << endl;
        for (Wizard const& wizard: his_wizards) {
            Snaffles::iterator it = find_if(snaffles.begin(), snaffles.end(), [&](Snaffle const& snaffle) {
                return wizard.contains(snaffle);
            });
            if (it != snaffles.end())
                snaffles.erase(it);
        }

        cerr << "after removing: " << snaffles.size() << endl;
        for (Wizard const& wizard: my_wizards) {
            if (wizard.state == 1) {
                if (teamId == 0)
                    cout << "THROW " << 16000 << " " << 3750 << " 500" << endl;
                else
                    cout << "THROW " << 0 << " " << 3750 << " 500" << endl;
                continue;
            }

            bool casted = false;
            for (auto const& bludger : bludgers) {
                if (bludger.target)
                    cerr << "Analyzing wizard: " << wizard.id << " bludger: " << bludger.id << " target: " << bludger.target->id << " dist: " << bludger.dist << endl;
                if (bludger.target && bludger.target->id == wizard.id && mana > 10 && bludger.dist < ((wizard.r + bludger.r) * 2.5f)) {
                    cout << "PETRIFICUS " << bludger.id << endl;
                    mana -= 10;
                    casted = true;
                    break;
                }
            }
            if (casted) continue;

            if (snaffles.empty()){
                cout << "MOVE " << his_wizards[0].x << " " << his_wizards[0].y << " 150" << endl;
                continue;
            }

            using SnaffleDist = pair<float, Snaffle>;
            vector<SnaffleDist> dists;
            transform(snaffles.begin(), snaffles.end(), back_inserter(dists),  [&](Snaffle const& snaffle){
                return SnaffleDist(wizard.distance(snaffle), snaffle);
            });
            sort(dists.begin(), dists.end(), [](SnaffleDist const& a, SnaffleDist const& b){
                return a.first < b.first;
            });

            cerr << snaffles.size() << endl;
            cout << "MOVE " << dists[0].second.x << " " << dists[0].second.y << " 150" << endl;
            snaffles.erase(find_if(snaffles.begin(), snaffles.end(), [&](Snaffle const& snaffle) {
                return dists[0].second.id == snaffle.id;
            }));
        }

        ++mana;
        clear();
    }
}
