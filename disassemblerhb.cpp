#include "utilityFunc.h"

int main ( int argc, char *argv[] )
{
    if ( argc != 2 ) {
        cout<< "There should be 1 argument!";
        return 0;
    }

    ifstream the_file ( argv[1], ios::in | ios::binary );
    if (the_file.fail())
    {
        cout << "Could not process " << argv[1] << "!\n";
        exit(1);
    }
    string contents(istreambuf_iterator<char>(the_file), (istreambuf_iterator<char>()));

    if(!contents.size()) {
        cout<< "The file is empty";
        return 0;
    }

    string name;
    for(int i=0; argv[1][i]; i++){
        if(argv[1][i] == '.') break;
        name += argv[1][i];
    }
    openOutputFile(name);
    createMaps();
    getLabels(contents);
    process(contents);
}
