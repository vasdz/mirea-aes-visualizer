#pragma once

#include <QMainWindow>
#include <array>
#include <vector>
#include <cstdint>
#include <span>
#include <QCloseEvent> 

class QTableWidget;
class QLineEdit;
class QPushButton;
class QTextBrowser;
class QComboBox;
class QProgressBar;

class MainWindow : public QMainWindow {
Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onEncryptClicked();
    void onNextStep();
    void onPrevStep();

private:
    using State = std::array<std::array<uint8_t, 4>, 4>;
    struct Step {
        QString name;
        State state;
        QString description;
    };

    void applyDarkTheme();
    void setupUi();
    void generateRandomKey();

    // AES Core Functions
    void keyExpansion(const std::vector<uint8_t>& key, std::vector<uint8_t>& roundKeys);
    void subBytes(State& state);
    void shiftRows(State& state);
    void mixColumns(State& state);
    void addRoundKey(State& state, const std::vector<uint8_t>& roundKeys, int round);

    void setupEncryptPage(QWidget* page);
    void setupTheoryPage(QWidget* page);
    void setupModesPage(QWidget* page);

    void updateStateView();
    void updateTheoryView();

    QTableWidget* stateTable = nullptr;
    QLineEdit* plaintextEdit = nullptr;
    QLineEdit* keyEdit = nullptr;
    QComboBox* modeBox = nullptr;
    QProgressBar* progressBar = nullptr;
    QPushButton* encryptButton = nullptr;
    QPushButton* nextStepButton = nullptr;
    QPushButton* prevStepButton = nullptr;
    QTextBrowser* theoryView = nullptr;

    std::vector<Step> steps;
    int currentStepIndex = -1;
};
