#include <iostream>
#include <iomanip>

using namespace std;

#include <RAT/DS/MC.hh>
#include <RAT/DS/MCTrack.hh>
#include <RAT/DS/MCTrackStep.hh>
#include <RAT/DS/Root.hh>

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>

class Particle {
    public:
        Particle();
        virtual ~Particle();
        
        void setProperties(string name, string gen_process, string end_process, TVector3 *steps, int nSteps);
        void addChild(Particle *child);
        
        int numRemaining();
        int numChildren();
        
        Particle *getNext();
        Particle *getChildren();
        
        void dumpTree(ostream &out);
        void dumpList(ostream &out);
        
        string name;
        string gen_process;
        string end_process;
        TVector3 *steps; //take ownership of this
        int nSteps;

    protected:
        void dumpTree(int depth, ostream &out);
        
        //don't take ownership of any of these pointers
        Particle *next;
        Particle *children;
        Particle *child_tail;
};

Particle::Particle() : name("UNSET"), gen_process("UNSET"), end_process("UNSET"), next(NULL), children(NULL), child_tail(NULL), steps(NULL), nSteps(0) { 
}

Particle::~Particle() { 
    if (steps) delete steps;
}

void Particle::setProperties(string name, string gen_process, string end_process, TVector3 *steps, int nSteps) {
    this->name = name;
    this->gen_process = gen_process;
    this->end_process = end_process;
    this->steps = steps;
    this->nSteps = nSteps;
}
        
void Particle::addChild(Particle *child) {
    if (children) {
        child_tail->next = child;
    } else {
        children = child;
    }
    child_tail = child;
}

int Particle::numChildren() {
    if (children) return 1+children->numRemaining();
    return 0;
}

int Particle::numRemaining() {
    int remaining = 0;
    Particle *cur = this;
    while (cur->next) {
        remaining++;
        cur = cur->next;
    }
    return remaining;
}

Particle *Particle::getNext() {
    return next;
}

Particle *Particle::getChildren() {
    return children;
}

void Particle::dumpList(ostream &out) {
    out << fixed << setprecision(5);
    out << '{';
        out << '\"' << name << "\",";
        out << '\"' << gen_process << "\",";
        out << '\"' << end_process << "\",";
        out << '{';
            for (int i = 0; i < nSteps-1; i++) {
                out << '{' << steps[i].X() << ',' << steps[i].Y() << ',' << steps[i].Z() << "},";
            }
            out << '{' << steps[nSteps-1].X() << ',' << steps[nSteps-1].Y() << ',' << steps[nSteps-1].Z() << "}";
        out << "},";
        out << '{';
            for (Particle *child = children; child; child = child->next) {
                child->dumpList(out); 
            }
        out << '}';
    out << '}';
    if (next) out << ",\n";
}

void Particle::dumpTree(ostream &out) {
    dumpTree(0,out);        
}

void Particle::dumpTree(int depth, ostream &out) {
    for (int i = 0; i < depth; i++) {
        out << '|';
    }
    out << name << ' '; 
    out << gen_process << ' ';
    out << end_process << ' ';
    out << '\n';
    for (Particle *child = children; child; child = child->next) {
        child->dumpTree(depth+1,out);
    }
}

void extractTree(const char *file, int event_num) {
    TFile *f = new TFile(file);
    TTree *tree = (TTree*) f->Get("T");
    
    RAT::DS::Root *rds = new RAT::DS::Root();
    tree->SetBranchAddress("ds", &rds);
    
    tree->GetEntry(event_num);
    RAT::DS::MC *mc = rds->GetMC();

    int nTracks = mc->GetMCTrackCount();
    Particle *trackmap = new Particle[nTracks+1];
    for (int j = 0; j < nTracks; j++) {
        RAT::DS::MCTrack *track = mc->GetMCTrack(j);
        int tid = track->GetID();
        int pid = track->GetParentID();
        
        Particle *cur = &trackmap[tid];
        trackmap[pid].addChild(cur);

        RAT::DS::MCTrackStep *first = track->GetMCTrackStep(0);
        RAT::DS::MCTrackStep *last = track->GetLastMCTrackStep();
        
        int nSteps = track->GetMCTrackStepCount();
        TVector3 *steps = new TVector3[nSteps];
        for (int k = 0; k < nSteps; k++) {
            steps[k] = track->GetMCTrackStep(k)->GetEndpoint();
        }

        cur->setProperties(track->GetParticleName(),first->GetProcess(),last->GetProcess(),steps,nSteps);
    }
    
    trackmap[1].dumpList(cout);
}

void extractEvents(const char *file) {
    TFile *f = new TFile(file);
    TTree *tree = (TTree*) f->Get("T");
    
    RAT::DS::Root *rds = new RAT::DS::Root();
    tree->SetBranchAddress("ds", &rds);
    
    int nEvents = tree->GetEntries();
    
    cout << '{';
    for (int i = 0; i < nEvents; i++) {
        tree->GetEntry(i);
        RAT::DS::MC *mc = rds->GetMC();
        RAT::DS::MCParticle *prim = mc->GetMCParticle(0);
        
        cout << '{';
            cout << mc->GetNumPE() << ',';
            cout << mc->GetMCPMTCount() << ',';
            cout << prim->GetKE() << ',';
            cout << '{' << prim->GetPosition().X() << ',' << prim->GetPosition().Y() << ',' << prim->GetPosition().Z() << "},";
            cout << '{' << prim->GetMomentum().X() << ',' << prim->GetMomentum().Y() << ',' << prim->GetMomentum().Z() << "}";
        cout << '}';
        if (i != nEvents-1) cout << ',';   
    }
    cout << "}\n";
}

int main(int argc, const char **argv) {

    if (argc == 2) {
        extractEvents(argv[1]);
    } else if (argc == 3) {
        extractTree(argv[1],atoi(argv[2]));
    } else {
        cout << "./event_dump root_file [event_num]\n";
        return 1;
    }
    
    return 0;
}
