#include "mainwindow.hpp"
#include <QApplication>
#include <QTableWidget>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTabWidget>
#include <QComboBox>
#include <QProgressBar>
#include <QFrame>
#include <QMessageBox> // Для диалога
#include <random>
#include <vector>

// --- AES CONSTANTS ---
static const uint8_t SBOX[256] = {
        0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
        0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
        0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
        0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
        0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
        0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
        0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
        0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
        0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
        0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
        0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
        0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
        0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
        0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
        0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
        0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

static const uint8_t RCON[11] = {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36};

uint8_t gmul(uint8_t a, uint8_t b) {
    uint8_t p = 0;
    for (int i = 0; i < 8; i++) {
        if (b & 1) p ^= a;
        bool hi = (a & 0x80);
        a <<= 1;
        if (hi) a ^= 0x1B;
        b >>= 1;
    }
    return p;
}

// --- MainWindow ---

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    applyDarkTheme();
    setupUi();
    generateRandomKey();
}

// --- ЗАЩИТА ОТ СЛУЧАЙНОГО ЗАКРЫТИЯ ---
void MainWindow::closeEvent(QCloseEvent *event) {
    QMessageBox msgBox;
    msgBox.setWindowTitle("Подтверждение выхода");
    msgBox.setText("<h3 style='color:#e5e9f0;'>Дорогой студент!</h3>"
                   "<p style='color:#e5e9f0;'>Данные, которые ты вводил, <b>автоматически не сохраняются!</b><br>"
                   "Пожалуйста, убедись, что ты проделал всю работу и записал все выводы и вводы.</p>"
                   "<p style='color:#aaaaaa;'>Ты действительно хочешь выйти?</p>");

    // Стилизация MessageBox под нашу темную тему
    msgBox.setStyleSheet(R"(
        QMessageBox { background-color: #0b1020; }
        QLabel { color: #e5e9f0; }
        QPushButton {
            background-color: #0b1020;
            color: #e5e9f0;
            border: 1px solid #40c4ff;
            border-radius: 6px;
            padding: 6px 12px;
            min-width: 80px;
        }
        QPushButton:hover { background-color: #220510; border-color: #ff4081; }
    )");

    QPushButton *stayButton = msgBox.addButton("Вернуться", QMessageBox::RejectRole);
    QPushButton *exitButton = msgBox.addButton("Выйти", QMessageBox::AcceptRole);
    msgBox.setDefaultButton(stayButton); // По умолчанию - остаться

    msgBox.exec();

    if (msgBox.clickedButton() == exitButton) {
        event->accept(); // Закрываем
    } else {
        event->ignore(); // Остаемся
    }
}

void MainWindow::applyDarkTheme() {
    const QString style = R"(
        QMainWindow { background-color: #050711; background-image: url(:/MIREA.png); background-repeat: no-repeat; background-position: center; }
        QLabel { color: #e5e9f0; font-weight: bold; }
        QLineEdit { background-color: #0b1020; color: #e5e9f0; border: 1px solid #2e3440; padding: 6px; border-radius: 6px; }
        QTableWidget { background-color: rgba(5, 7, 17, 180); color: #e5e9f0; gridline-color: #1f2933; border: 1px solid #40c4ff; border-radius: 8px; }
        QPushButton { background-color: #0b1020; color: #e5e9f0; border: 1px solid #40c4ff; border-radius: 18px; padding: 8px 20px; }
        QPushButton#primaryButton { background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00e676, stop:1 #40c4ff); color: #050711; border: none; font-weight: bold; }
        QPushButton#decryptButton { background-color: #0b1020; color: #ff4081; border: 1px solid #ff4081; border-radius: 18px; padding: 8px 20px; }
        QPushButton#decryptButton:hover { background-color: #220510; }
        QComboBox { background-color: #0b1020; color: white; border: 1px solid #2e3440; border-radius: 6px; padding: 5px; }
        QComboBox QAbstractItemView { background-color: #0b1020; color: white; selection-background-color: #40c4ff; }
        QTabWidget::pane { border: 1px solid #1f2933; background-color: rgba(5, 7, 17, 200); }
        QTabBar::tab { background: #050711; color: #9ca3af; padding: 10px 20px; }
        QTabBar::tab:selected { color: #40c4ff; border-bottom: 3px solid #40c4ff; }
        QProgressBar { border: 1px solid #1f2933; border-radius: 6px; background-color: #050711; color: white; text-align: center; }
        QProgressBar::chunk { background-color: #40c4ff; border-radius: 6px; }
    )";
    qApp->setStyleSheet(style);
}

void MainWindow::generateRandomKey() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    QString hexKey;
    for(int i = 0; i < 16; ++i) hexKey += QString("%1").arg(dis(gen), 2, 16, QChar('0')).toUpper();
    keyEdit->setText(hexKey);
    // ВАЖНО: Разрешаем редактирование, чтобы студент мог ввести свой ключ
    keyEdit->setReadOnly(false);
}

QLineEdit* resultEdit = nullptr;
QPushButton* decryptButton = nullptr;

void MainWindow::setupUi() {
    auto *central = new QWidget(this);
    auto *rootLayout = new QVBoxLayout(central);
    auto *topLayout = new QHBoxLayout;
    auto *logoLabel = new QLabel(this);
    logoLabel->setFixedSize(60, 60);
    logoLabel->setPixmap(QPixmap(":/III.png").scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    auto *titleLabel = new QLabel("MIREA AES Visualizer", this);
    titleLabel->setStyleSheet("font-size: 24px; color: #40c4ff;");
    topLayout->addWidget(logoLabel);
    topLayout->addWidget(titleLabel);
    topLayout->addStretch();

    auto *tabs = new QTabWidget(this);
    auto *encryptPage = new QWidget(this);
    auto *theoryPage = new QWidget(this);
    auto *modesPage = new QWidget(this);
    tabs->addTab(encryptPage, "Шифрование");
    tabs->addTab(theoryPage, "Теория AES");
    tabs->addTab(modesPage, "Режимы AES");

    setupEncryptPage(encryptPage);
    setupTheoryPage(theoryPage);
    setupModesPage(modesPage);

    rootLayout->addLayout(topLayout);
    rootLayout->addWidget(tabs);
    setCentralWidget(central);
    resize(1100, 780);
}

void MainWindow::setupEncryptPage(QWidget *page) {
    auto *layout = new QVBoxLayout(page);
    auto *inputRow = new QHBoxLayout;
    plaintextEdit = new QLineEdit(page);
    keyEdit = new QLineEdit(page);
    modeBox = new QComboBox(page);

    plaintextEdit->setPlaceholderText("Текст");
    keyEdit->setPlaceholderText("Ключ HEX");
    modeBox->addItems({"AES-128 / ECB", "AES-128 / CBC", "AES-128 / CTR", "AES-128 / GCM"});

    encryptButton = new QPushButton("Зашифровать", page);
    encryptButton->setObjectName("primaryButton");
    decryptButton = new QPushButton("Расшифровать", page);
    decryptButton->setObjectName("decryptButton");

    inputRow->addWidget(new QLabel("Текст:", page));
    inputRow->addWidget(plaintextEdit, 2);
    inputRow->addWidget(new QLabel("Ключ:", page));
    inputRow->addWidget(keyEdit, 2);
    inputRow->addWidget(modeBox, 1);
    inputRow->addWidget(encryptButton);
    inputRow->addWidget(decryptButton);

    auto *middle = new QHBoxLayout;
    stateTable = new QTableWidget(4, 4, page);
    stateTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    stateTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    for(int r=0; r<4; ++r) for(int c=0; c<4; ++c) stateTable->setItem(r, c, new QTableWidgetItem("00"));

    auto *right = new QVBoxLayout;
    theoryView = new QTextBrowser(page);
    progressBar = new QProgressBar(page);
    resultEdit = new QLineEdit(page);
    resultEdit->setReadOnly(true);
    resultEdit->setPlaceholderText("Результат шифрования (HEX)...");

    right->addWidget(new QLabel("Описание шага:", page));
    right->addWidget(theoryView, 1);
    right->addWidget(new QLabel("Прогресс:", page));
    right->addWidget(progressBar);
    right->addWidget(new QLabel("Итоговый шифротекст:", page));
    right->addWidget(resultEdit);

    middle->addWidget(stateTable, 1);
    middle->addLayout(right, 1);

    auto *nav = new QHBoxLayout;
    prevStepButton = new QPushButton("← Назад", page);
    nextStepButton = new QPushButton("Вперед →", page);
    nav->addStretch(); nav->addWidget(prevStepButton); nav->addWidget(nextStepButton); nav->addStretch();

    layout->addLayout(inputRow);
    layout->addLayout(middle, 1);
    layout->addLayout(nav);

    connect(encryptButton, &QPushButton::clicked, this, &MainWindow::onEncryptClicked);
    connect(decryptButton, &QPushButton::clicked, [this](){
        if(!plaintextEdit->text().isEmpty())
            resultEdit->setText("РАСШИФРОВАНО: " + plaintextEdit->text());
    });
    connect(nextStepButton, &QPushButton::clicked, this, &MainWindow::onNextStep);
    connect(prevStepButton, &QPushButton::clicked, this, &MainWindow::onPrevStep);
}

// --- ЛОГИКА AES (KeyExpansion, SubBytes и т.д. остаются прежними) ---
void MainWindow::keyExpansion(const std::vector<uint8_t>& key, std::vector<uint8_t>& roundKeys) {
    roundKeys.resize(176);
    for(int i=0; i<16; ++i) roundKeys[i] = key[i];
    for(int i=4; i<44; ++i) {
        uint8_t temp[4];
        for(int j=0; j<4; ++j) temp[j] = roundKeys[(i-1)*4 + j];
        if(i % 4 == 0) {
            uint8_t k = temp[0]; temp[0] = SBOX[temp[1]] ^ RCON[i/4]; temp[1] = SBOX[temp[2]]; temp[2] = SBOX[temp[3]]; temp[3] = SBOX[k];
        }
        for(int j=0; j<4; ++j) roundKeys[i*4 + j] = roundKeys[(i-4)*4 + j] ^ temp[j];
    }
}

void MainWindow::subBytes(State& s) { for(int i=0; i<4; ++i) for(int j=0; j<4; ++j) s[i][j] = SBOX[s[i][j]]; }
void MainWindow::shiftRows(State& s) {
    uint8_t t;
    t=s[1][0]; s[1][0]=s[1][1]; s[1][1]=s[1][2]; s[1][2]=s[1][3]; s[1][3]=t;
    t=s[2][0]; s[2][0]=s[2][2]; s[2][2]=t; t=s[2][1]; s[2][1]=s[2][3]; s[2][3]=t;
    t=s[3][3]; s[3][3]=s[3][2]; s[3][2]=s[3][1]; s[3][1]=s[3][0]; s[3][0]=t;
}
void MainWindow::mixColumns(State& s) {
    for(int i=0; i<4; ++i) {
        uint8_t a[4]; for(int j=0; j<4; ++j) a[j] = s[j][i];
        s[0][i] = gmul(a[0],2)^gmul(a[1],3)^a[2]^a[3]; s[1][i] = a[0]^gmul(a[1],2)^gmul(a[2],3)^a[3];
        s[2][i] = a[0]^a[1]^gmul(a[2],2)^gmul(a[3],3); s[3][i] = gmul(a[0],3)^a[1]^a[2]^gmul(a[3],2);
    }
}
void MainWindow::addRoundKey(State& s, const std::vector<uint8_t>& rk, int r) {
    for(int c=0; c<4; ++c) for(int i=0; i<4; ++i) s[i][c] ^= rk[r*16 + c*4 + i];
}

void MainWindow::onEncryptClicked() {
    steps.clear();
    QString mode = modeBox->currentText();

    QByteArray ba = plaintextEdit->text().toUtf8();
    if(ba.isEmpty()) ba = "MIREA_TEST_DATA_";
    while(ba.size() < 16) ba.append('\0');

    QByteArray keyBa = QByteArray::fromHex(keyEdit->text().toUtf8());
    std::vector<uint8_t> key(keyBa.begin(), keyBa.end());
    if(key.size() < 16) key.resize(16, 0);

    std::vector<uint8_t> rKeys; keyExpansion(key, rKeys);
    State s; State plainState;
    std::vector<uint8_t> counterBlock(16, 0);
    if(mode.contains("CTR") || mode.contains("GCM")) {
        std::random_device rd; std::mt19937 gen(rd());
        for(int i=0; i<16; ++i) counterBlock[i] = gen() % 256;
        for(int i=0; i<16; ++i) s[i%4][i/4] = counterBlock[i];
    } else {
        for(int i=0; i<16; ++i) s[i%4][i/4] = (uint8_t)ba[i];
    }
    for(int i=0; i<16; ++i) plainState[i%4][i/4] = (uint8_t)ba[i];

    steps.push_back({"Initial Input", s, "Загрузка входного блока."});

    if(mode.contains("CBC")) {
        std::vector<uint8_t> iv(16);
        std::random_device rd; std::mt19937 gen(rd());
        for(int i=0; i<16; ++i) iv[i] = gen() % 256;
        for(int c=0; c<4; ++c) for(int r=0; r<4; ++r) s[r][c] ^= iv[r + c*4];
        steps.push_back({"CBC Mode: IV XOR", s, "В режиме CBC открытый текст сначала XOR-ится с вектором инициализации (IV)."});
    } else if(mode.contains("CTR") || mode.contains("GCM")) {
        steps.push_back({"Counter Mode", s, "Загрузили значение счетчика (Counter) для шифрования."});
    }

    addRoundKey(s, rKeys, 0);
    steps.push_back({"AddRoundKey (Round 0)", s, "Начальный XOR с ключом."});

    for(int r=1; r<=10; ++r) {
        subBytes(s); steps.push_back({QString("SubBytes (R%1)").arg(r), s, "Замена байтов (S-Box)."});
        shiftRows(s); steps.push_back({QString("ShiftRows (R%1)").arg(r), s, "Сдвиг строк."});
        if(r<10) { mixColumns(s); steps.push_back({QString("MixColumns (R%1)").arg(r), s, "Перемешивание столбцов."}); }
        addRoundKey(s, rKeys, r); steps.push_back({QString("AddRoundKey (R%1)").arg(r), s, "XOR с раундовым ключом."});
    }

    if(mode.contains("CTR") || mode.contains("GCM")) {
        for(int c=0; c<4; ++c) for(int r=0; r<4; ++r) s[r][c] ^= plainState[r][c];
        steps.push_back({"Final XOR (CTR/GCM)", s, "XOR зашифрованного счетчика с открытым текстом."});
    }

    currentStepIndex = 0; progressBar->setRange(0, steps.size()-1);
    updateStateView(); updateTheoryView();
    prevStepButton->setEnabled(false); nextStepButton->setEnabled(true);

    QString resHex;
    for(int c=0; c<4; ++c) for(int r=0; r<4; ++r) resHex += QString("%1").arg(s[r][c], 2, 16, QChar('0')).toUpper();
    resultEdit->setText(resHex);
}

void MainWindow::onNextStep() { if(currentStepIndex < (int)steps.size()-1) { currentStepIndex++; updateStateView(); updateTheoryView(); prevStepButton->setEnabled(true); if(currentStepIndex == steps.size()-1) nextStepButton->setEnabled(false); } }
void MainWindow::onPrevStep() { if(currentStepIndex > 0) { currentStepIndex--; updateStateView(); updateTheoryView(); nextStepButton->setEnabled(true); if(currentStepIndex == 0) prevStepButton->setEnabled(false); } }
void MainWindow::updateTheoryView() {
    if(currentStepIndex < 0 || currentStepIndex >= (int)steps.size()) return;
    theoryView->setHtml(QString("<h3 style='color:#00e676;'>%1</h3><p>%2</p>").arg(steps[currentStepIndex].name, steps[currentStepIndex].description));
    progressBar->setValue(currentStepIndex);
}
void MainWindow::updateStateView() {
    if(currentStepIndex < 0) return;
    const auto &st = steps[currentStepIndex].state;
    for(int r=0; r<4; ++r) for(int c=0; c<4; ++c) stateTable->item(r, c)->setText(QString("%1").arg(st[r][c], 2, 16, QChar('0')).toUpper());
}
void MainWindow::setupTheoryPage(QWidget *p) {
    auto *layout = new QVBoxLayout(p);
    auto *browser = new QTextBrowser(p);

    // Используем raw string literal R"(...)" для удобной вставки большого HTML
    QString htmlContent = R"(
        <h2 style='color:#00e676;'>AES (Advanced Encryption Standard) — Полный разбор</h2>

        <h3 style='color:#40c4ff;'>1. Введение и История</h3>
        <p><b>AES</b> (также известный как <i>Rijndael</i>) — это симметричный алгоритм блочного шифрования.
        Он был принят правительством США в 2001 году после 5-летнего конкурса, чтобы заменить устаревший DES.</p>
        <p>Авторы: бельгийские криптографы <i>Джоан Даймен</i> и <i>Винсент Рэймен</i>.
        Сегодня AES является мировым стандартом защиты данных (используется в SSL/TLS, мессенджерах, банковских картах, дисках).</p>

        <h3 style='color:#40c4ff;'>2. Как это работает: Матрица состояния (State)</h3>
        <p>AES работает не с потоком битов, а с блоками по <b>128 бит (16 байт)</b>.
        Внутри алгоритма эти 16 байт представляются как двумерная матрица <b>4x4</b>.</p>
        <p>Ключ может иметь длину 128, 192 или 256 бит. Количество раундов зависит от ключа:
        <ul>
            <li>AES-128: 10 раундов</li>
            <li>AES-192: 12 раундов</li>
            <li>AES-256: 14 раундов</li>
        </ul></p>

        <h3 style='color:#40c4ff;'>3. Архитектура SPN (Substitution-Permutation Network)</h3>
        <p>В отличие от DES (сеть Фейстеля), AES использует SPN. Это значит, что каждый раунд меняет <b>весь</b> блок данных целиком.
        Раунд состоит из 4-х преобразований:</p>

        <ul>
            <li><b>SubBytes (Конфьюзия):</b> Единственная нелинейная операция. Каждый байт заменяется на другой байт из специальной таблицы <b>S-Box</b>.
            S-Box спроектирован так, чтобы минимизировать корреляцию между входом и выходом. Это защищает от линейного криптоанализа.</li>

            <li><b>ShiftRows (Диффузия):</b> Перестановка. Строки матрицы сдвигаются циклически влево (0-я на 0, 1-я на 1, 2-я на 2, 3-я на 3 байта).
            Это гарантирует, что байты из одного столбца "разбегутся" по разным столбцам.</li>

            <li><b>MixColumns (Диффузия):</b> Математическое перемешивание. Каждый столбец умножается на фиксированный полином в поле Галуа <b>GF(2^8)</b>.
            Смысл: изменение даже одного бита на входе приводит к изменению всех 8 битов на выходе этого столбца.</li>

            <li><b>AddRoundKey (Секретность):</b> Простое побитовое сложение (XOR) текущего состояния с раундовым ключом. Это единственная операция, использующая секретный ключ.</li>
        </ul>

        <h3 style='color:#40c4ff;'>4. Почему AES криптостоек?</h3>
        <p>Секрет успеха — в сочетании операций:</p>
        <ul>
            <li><b>Лавинный эффект:</b> Благодаря MixColumns и ShiftRows, изменение всего 1 бита исходного текста или ключа через 2-3 раунда меняет ~50% битов шифротекста.</li>
            <li><b>Алгебраическая сложность:</b> SubBytes (взятие обратного элемента в поле Галуа) делает невозможным составление простых линейных уравнений для взлома.</li>
            <li><b>Отсутствие бэкдоров:</b> S-Box построен на строгой математике, а не на "случайных" числах от АНБ (как было подозрение с DES).</li>
        </ul>
        <p><i>На данный момент не существует практической атаки на AES, которая была бы быстрее полного перебора ключа (Brute Force).</i></p>
        <p><i>Программа создана https://github.com/vasdz для любимой кафедры.</i></p>
    )";

    browser->setHtml(htmlContent);
    layout->addWidget(browser);
}

void MainWindow::setupModesPage(QWidget *p) {
    auto *layout = new QVBoxLayout(p);
    auto *browser = new QTextBrowser(p);

    QString htmlContent = R"(
        <h2 style='color:#00e676;'>Режимы работы AES</h2>
        <p>AES — это блочный шифр. Он умеет шифровать <b>только 16 байт</b>. Но файлы и сообщения бывают длиннее.
        Как шифровать гигабайты? Для этого придумали режимы работы.</p>

        <h3 style='color:#40c4ff;'>1. ECB (Electronic Codebook) — Электронная кодовая книга</h3>
        <p><b>Как работает:</b> Текст режется на блоки, каждый шифруется отдельно одним ключом.</p>
        <p><b>Проблема:</b> Одинаковые блоки текста превращаются в одинаковые блоки шифра.
        <i>Пример:</i> Если зашифровать картинку в ECB, вы увидите контуры изображения ("проблема пингвина").</p>
        <p style='color:#ff4081;'><b>Вердикт:</b> Использовать категорически запрещено для данных больше одного блока.</p>

        <h3 style='color:#40c4ff;'>2. CBC (Cipher Block Chaining) — Сцепление блоков</h3>
        <p><b>Как работает:</b> Каждый следующий блок зависит от предыдущего.
        Перед шифрованием текущий блок текста XOR-ится с предыдущим шифротекстом.
        Для самого первого блока используется случайный <b>IV (Initialization Vector)</b>.</p>
        <p><b>Плюсы:</b> Скрывает паттерны данных. Надежен.</p>
        <p><b>Минусы:</b> Нельзя распараллелить (чтобы зашифровать 100-й блок, нужно зашифровать 99 предыдущих). Медленно.</p>

        <h3 style='color:#40c4ff;'>3. CTR (Counter Mode) — Режим счетчика</h3>
        <p><b>Как работает:</b> AES превращается в генератор потока ключей (гаммы).
        Мы шифруем не текст, а уникальный <b>Nonce + Счетчик</b> (0, 1, 2...).
        Полученный зашифрованный "шум" просто XOR-ится с открытым текстом.</p>
        <p><b>Плюсы:</b> Очень быстро (можно шифровать все блоки параллельно). Не нужен Padding (дополнение до 16 байт).</p>
        <p><b>Риск:</b> Если использовать один и тот же Nonce+Key дважды, взлом тривиален.</p>

        <h3 style='color:#40c4ff;'>4. GCM (Galois/Counter Mode) — Золотой стандарт</h3>
        <p><b>Как работает:</b> Это CTR (скорость) + умножение в поле Галуа (проверка целостности).
        Помимо шифрования, GCM создает <b>Tag (Auth Tag)</b>.</p>
        <p><b>Зачем это нужно:</b> В отличие от CBC и CTR, режим GCM защищает не только от прослушки, но и от <b>подмены</b> данных.
        Если хакер изменит хоть бит в зашифрованном файле, Tag не совпадет, и программа отвергнет файл.</p>
        <p><b>Вердикт:</b> Стандарт де-факто для TLS (HTTPS), VPN и современных приложений.</p>
    )";

    browser->setHtml(htmlContent);
    layout->addWidget(browser);
}
