#ifndef PREFERENCES_H
#define PREFERENCES_H

namespace Ori {
class Settings;
}

class Preferences
{
private:
    Preferences();
    static Preferences preferences;

public:
    static Preferences& instance();
    static Preferences* instancePtr();

    bool bugEditorEnableDates;
    bool bugSolverEnableDates;
    bool confirmCancel;
    bool openNewBugOnPage;

    void load(Ori::Settings& s);
    void save(Ori::Settings& s);
    void save();
};

#endif // PREFERENCES_H
