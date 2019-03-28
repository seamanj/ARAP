#ifndef PICKER_H
#define PICKER_H

#include <QList>

class Picker
{

public:

    static const unsigned int NO_PICK;

    Picker( int n );
    ~Picker();

    void setObjectIndex( unsigned int i ) const;

    unsigned int getPick();
    QList<unsigned int> getPicks();

private:

    struct PickRecord {
        unsigned int numNames;
        unsigned int minDepth;
        unsigned int maxDepth;
        unsigned int name;
    };

    bool            m_selectMode;

    int             m_nObjects;
    PickRecord     *m_picks;

};

#endif // PICKER_H
