#include<bits/stdc++.h>
using namespace std;

int PC = 0;

map<int,  string> opcodes;  // opcode  ,  type/instruction "r" --> R , "i" --> I , "s" --> S , "b" --> B , "j" --> jalr , "f" --> fence
map<pair<int, string>, string> func3;  // <func3,  type>  ,  sub-type/instruction
map<pair<int, string>, string> func7;    //  <func7,  sub-type>  ,  instruction

// creating the maps to encode the instructions from files
void createMaps()
{
    int o;
    string c, s;

    ifstream opf;
    opf.open("opcodes.txt");
    while(!opf.eof()){
        opf>>o>>s;
        opcodes[o] = s;
    }
    opf.close();

    ifstream func3f;
    func3f.open("func3.txt");
    while(!func3f.eof()){
        func3f>>o>>c>>s;
        func3[{o,c}] = s;
    }
    func3f.close();

    ifstream func7f;
    func7f.open("func7.txt");
    while(!func7f.eof()){
        func7f>>o>>c>>s;
        func7[{o,c}] = s;
    }
    func7f.close();

    for(auto i:func7) cout<< i.first.first << " " << i.first.second << " " << i.second<<endl;
}

void process(string& obj)
{
    int i=0;
    for(;;){
        char cur = obj[i];

        // check if it is compressed instruction or not

    }
}

int main ( int argc, char *argv[] )
{
    /*
    if ( argc != 2 ) {
        cout<< "There should be 1 argument!";
        return 0;
    }

    ifstream the_file ( argv[1], ios::in | ios::binary );
    if (the_file.fail())
    {
        cout << "Could not process " << argv[1] << "!\n";
        exit(1);
    }*/
    ifstream the_file ( "div.bin", ios::in | ios::binary );
    string contents(istreambuf_iterator<char>(the_file), (istreambuf_iterator<char>()));

    if(!contents.size()) {
        cout<< "The file is empty";
        return 0;
    }

    createMaps();
    process(contents);
}
