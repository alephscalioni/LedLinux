#include <QtWidgets>
#include <QLocalServer>
#include <QLocalSocket>

static QIcon swatch(QColor color) {
    QPixmap pix(32,32); pix.fill(Qt::transparent);
    QPainter p(&pix); p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor("#f0f0f0"),2)); p.setBrush(color); p.drawEllipse(QRectF(5,5,22,22));
    return QIcon(pix);
}

int main(int argc, char **argv) {
    // XWayland permits exact tray anchoring for this small Qt popup.
    if(qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) qputenv("QT_QPA_PLATFORM", "xcb");
    QApplication app(argc,argv);
    app.setApplicationName("led-control");
    app.setApplicationDisplayName("Cores dos LEDs");
    app.setOrganizationName("Aleph");
    app.setQuitOnLastWindowClosed(false);
    QFont textFont("SF Pro Text"); textFont.setPixelSize(13);
    textFont.setWeight(QFont::Normal); textFont.setKerning(true);
    textFont.setLetterSpacing(QFont::AbsoluteSpacing, 0.15);
    app.setFont(textFont);
    const bool checkSync=app.arguments().contains("--check-sync");
    const int captureArg=app.arguments().indexOf("--screenshot");
    const bool capture=captureArg>=0 && captureArg+1<app.arguments().size();
    const QString serverName=(checkSync||capture)?QString("aleph-led-control-check-%1").arg(QCoreApplication::applicationPid()):QString("aleph-led-control");
    QString base=QDir::homePath()+"/.local/share/led-control";
    QLocalSocket existing; existing.connectToServer(serverName);
    if(existing.waitForConnected(250)) { existing.write("show"); existing.flush(); existing.waitForBytesWritten(250); return 0; }
    QLocalServer::removeServer(serverName);
    QLocalServer server; server.listen(serverName);
    QSettings settings;
    QColor current(settings.value("color","#0000ff").toString());
    if(!current.isValid()) current=QColor("#0000ff");
    const bool m87Enabled=settings.value("m87Enabled", false).toBool();
    QSystemTrayIcon tray(swatch(current));
    tray.setToolTip("Cores dos LEDs");
    QMenu menu;
    auto title=menu.addAction("Cores dos LEDs"); title->setEnabled(false);
    auto statusAction=menu.addAction("Preparando iluminação…"); statusAction->setEnabled(false);
    menu.addSeparator();
    QFrame window(nullptr, Qt::Popup | Qt::FramelessWindowHint);
    window.setObjectName("ledPopover"); window.setWindowTitle("Cores dos LEDs"); window.setFixedWidth(390);
    window.setStyleSheet("QFrame#ledPopover {background: #222732; border: 1px solid #50596b; border-radius: 16px;} QLabel {color: #f3f5fa;} QPushButton {padding: 8px 6px; border-radius: 8px; background: #363e4e; color: #f3f5fa; border: none;} QPushButton:hover {background: #46536a;} QPushButton:focus {border: 1px solid #8bbcff;} QCheckBox {color: #d6dce7;} QToolButton {color: #d6dce7; background: #222732; border: none; padding: 4px 0;} QScrollArea {background: #222732; border: none;} QScrollBar:vertical {background: #222732; width: 10px;} QScrollBar::handle:vertical {background: #50596b; min-height: 24px; border-radius: 4px;} QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {height: 0;} QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {background: none;}");
    auto outerLayout=new QVBoxLayout(&window);outerLayout->setContentsMargins(0,0,0,0);
    auto body=new QWidget;body->setObjectName("popoverBody");body->setStyleSheet("QWidget#popoverBody {background: #222732;}");
    auto bodyScroll=new QScrollArea;bodyScroll->setWidgetResizable(true);bodyScroll->setFrameShape(QFrame::NoFrame);bodyScroll->setWidget(body);outerLayout->addWidget(bodyScroll);
    auto layout=new QVBoxLayout(body); layout->setContentsMargins(20,20,20,20); layout->setSpacing(12);
    auto heading=new QLabel("Cores dos LEDs"); QFont f=heading->font(); f.setPixelSize(16); f.setWeight(QFont::DemiBold); f.setLetterSpacing(QFont::AbsoluteSpacing, 0.1); heading->setFont(f); layout->addWidget(heading);
    auto description=new QLabel("Iluminação sincronizada"); description->setWordWrap(true); layout->addWidget(description);
    auto preview=new QPushButton; preview->setMinimumHeight(38); layout->addWidget(preview);
    auto presets=new QGridLayout; presets->setSpacing(8); layout->addLayout(presets);
    auto custom=new QPushButton("Escolher outra cor…"); layout->addWidget(custom);
    auto brightnessLabel=new QLabel("Brilho"); layout->addWidget(brightnessLabel);
    auto brightness=new QSlider(Qt::Horizontal); brightness->setRange(0,100); brightness->setValue(settings.value("brightness",100).toInt()); brightness->setAccessibleName("Brilho dos LEDs"); layout->addWidget(brightness);
    auto effectRow=new QWidget; auto effectLayout=new QFormLayout(effectRow); effectLayout->setContentsMargins(0,0,0,0);
    auto globalEffect=new QComboBox;
    for(auto item:QList<QPair<QString,QString>>{{"Estático","static"},{"Respiração","breathing"},{"Ciclo de cores RGB","spectrum"},{"Onda RGB","wave"},{"Rastro","chase"},{"Transição de cores","color_shift"},{"Desligado","off"}}) globalEffect->addItem(item.first,item.second);
    int globalIndex=globalEffect->findData(settings.value("globalEffect","breathing").toString());globalEffect->setCurrentIndex(globalIndex<0?1:globalIndex);
    globalEffect->setAccessibleName("Efeito em todos os LEDs");effectLayout->addRow("Todos os LEDs",globalEffect);layout->addWidget(effectRow);
    auto waterTitle=new QLabel("Water cooler · Aura Ice");QFont waterFont=waterTitle->font();waterFont.setWeight(QFont::DemiBold);waterTitle->setFont(waterFont);layout->addWidget(waterTitle);
    auto waterStats=new QLabel("Lendo temperaturas…");waterStats->setWordWrap(true);waterStats->setTextInteractionFlags(Qt::TextSelectableByMouse);layout->addWidget(waterStats);
    const QString waterConfig=QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)+"/rise-mode-aura-ice/display.json";
    auto waterSource=new QComboBox;waterSource->setAccessibleName("Temperatura enviada à telinha");
    waterSource->addItem("Tela: sensores originais","original");waterSource->addItem("Tela: temperatura da CPU","cpu_temperature");waterSource->addItem("Tela: temperatura da GPU","gpu_temperature");
    QFile waterConfigFile(waterConfig);if(waterConfigFile.open(QIODevice::ReadOnly)){int selected=waterSource->findData(QJsonDocument::fromJson(waterConfigFile.readAll()).object().value("temperature_source").toString());if(selected>=0)waterSource->setCurrentIndex(selected);}
    waterSource->setToolTip("Escolhe a fonte dos campos de temperatura. Não altera o modo ou a unidade do visor.");layout->addWidget(waterSource);
    int savedWaterIndex=waterSource->currentIndex();
    auto waterSourceError=new QLabel;waterSourceError->setWordWrap(true);waterSourceError->hide();layout->addWidget(waterSourceError);
    auto status=new QLabel; status->setWordWrap(true); status->setTextInteractionFlags(Qt::TextSelectableByMouse); layout->addWidget(status);
    auto details=new QToolButton; details->setText("Opções e dispositivos"); details->setCheckable(true); details->setToolButtonStyle(Qt::ToolButtonTextBesideIcon); details->setArrowType(Qt::RightArrow); layout->addWidget(details);
    auto advanced=new QWidget;advanced->setObjectName("advancedOptions");advanced->setStyleSheet("QWidget#advancedOptions {background: #222732;}"); auto advancedLayout=new QVBoxLayout(advanced); advancedLayout->setContentsMargins(0,0,0,0); advancedLayout->setSpacing(10); auto advancedScroll=new QScrollArea;advancedScroll->setWidgetResizable(true);advancedScroll->setFrameShape(QFrame::NoFrame);advancedScroll->setWidget(advanced);advancedScroll->setMinimumHeight(190);advancedScroll->setMaximumHeight(240);advancedScroll->hide();layout->addWidget(advancedScroll);
    auto devices=new QLabel; devices->setWordWrap(true); advancedLayout->addWidget(devices);
    QObject::connect(details,&QToolButton::toggled,[&](bool checked){ advancedScroll->setVisible(checked); details->setArrowType(checked?Qt::DownArrow:Qt::RightArrow); window.adjustSize();
        auto screen=window.screen();if(screen){auto area=screen->availableGeometry();window.setMaximumHeight(qMax(100,area.height()-16));window.resize(window.width(),qMin(body->sizeHint().height()+2,area.height()-16));window.move(qBound(area.left()+8,window.x(),qMax(area.left()+8,area.right()-window.width()-8)),qBound(area.top()+8,window.y(),qMax(area.top()+8,area.bottom()-window.height()-8)));}
    });
    auto globalSpeed=new QSlider(Qt::Horizontal);globalSpeed->setRange(0,100);globalSpeed->setValue(qBound(0,settings.value("globalSpeed",40).toInt(),100));globalSpeed->setAccessibleName("Velocidade global dos efeitos");
    advancedLayout->addWidget(new QLabel("Velocidade global · lento a rápido"));advancedLayout->addWidget(globalSpeed);
    auto waterState=new QLabel;waterState->setWordWrap(true);advancedLayout->addWidget(waterState);
    auto waterToggle=new QPushButton("Iniciar telinha");advancedLayout->addWidget(waterToggle);
    auto m87Independent=new QCheckBox("Efeito próprio no teclado");m87Independent->setChecked(settings.value("m87Independent",false).toBool());advancedLayout->addWidget(m87Independent);m87Independent->setVisible(m87Enabled);
    auto m87Mode=new QComboBox;
    m87Mode->addItems({"Desligado","Estático","Acender ao tocar","Apagar ao tocar","Cintilação","Queda","Colorido","Respiração","Espectro","Expansão","Rolagem horizontal","Rolagem vertical","Rotação","Explosão","Lançamento","Ondas","Fluxo","Pulsação","Inclinação","Travessia"});
    m87Mode->setCurrentIndex(qBound(0,settings.value("m87Mode",1).toInt(),19));m87Mode->setAccessibleName("Efeito próprio do teclado M87");
    auto m87Options=new QWidget; auto m87Form=new QFormLayout(m87Options);m87Form->setContentsMargins(0,0,0,0);m87Form->addRow("Efeito M87",m87Mode);
    auto m87Speed=new QSpinBox;m87Speed->setRange(0,5);m87Speed->setValue(qBound(0,settings.value("m87Speed",3).toInt(),5));
    m87Form->addRow("Velocidade M87",m87Speed);
    auto m87Direction=new QComboBox;m87Direction->addItems({"Normal","Inverso"});m87Direction->setCurrentIndex(qBound(0,settings.value("m87Direction",0).toInt(),1));
    m87Form->addRow("Sentido M87",m87Direction);
    auto m87Rainbow=new QCheckBox("Multicolorido no M87");m87Rainbow->setChecked(settings.value("m87Rainbow",false).toBool());m87Form->addRow(m87Rainbow);
    auto m87Note=new QLabel("Sem seleção própria, o M87 acompanha o efeito global. Lateral e setinha continuam pelo controle físico.");m87Note->setWordWrap(true);m87Note->setMinimumHeight(m87Note->fontMetrics().boundingRect(QRect(0,0,290,1000),Qt::TextWordWrap,m87Note->text()).height()+4);m87Form->addRow(m87Note);
    advancedLayout->addWidget(m87Options);m87Options->setVisible(m87Enabled);
    auto startup=new QCheckBox("Iniciar ao entrar na sessão");
    QString autostart=QDir::homePath()+"/.config/autostart/led-control.desktop";
    startup->setChecked(QFile::exists(autostart)); advancedLayout->addWidget(startup);
    auto footer=new QHBoxLayout; auto retry=new QPushButton("Reaplicar"); auto hide=new QPushButton("Pronto"); footer->addWidget(retry); footer->addStretch(); footer->addWidget(hide); layout->addLayout(footer);
    QObject::connect(hide,&QPushButton::clicked,&window,&QWidget::hide);
    QObject::connect(startup,&QCheckBox::toggled,[&](bool enabled) {
        if(enabled) { QDir().mkpath(QFileInfo(autostart).absolutePath()); QFile::copy(base+"/led-control.desktop",autostart); }
        else QFile::remove(autostart);
        if(QFile::exists(autostart)!=enabled) { status->setText("Não foi possível alterar o início automático."); QSignalBlocker block(startup); startup->setChecked(QFile::exists(autostart)); }
    });
    QProcess engine;engine.setProcessChannelMode(QProcess::SeparateChannels);
    engine.setStandardErrorFile(base+"/effects-engine.log",QIODevice::Append);
    QTimer engineTimeout;engineTimeout.setSingleShot(true);engineTimeout.setInterval(45000);
    QColor pending=current;
    int pendingBrightness=brightness->value();
    bool engineReady=false,engineBusy=false,shuttingDown=false;
    qint64 nextId=0;
    QJsonObject running,queued;
    QByteArray engineBuffer;
    auto setStatus=[&](QString message){status->setText(message);statusAction->setText(message);tray.setToolTip("Cores dos LEDs\n"+message);};
    auto updatePreview=[&]{preview->setIcon(swatch(pending));preview->setText(pending.name().toUpper()+" · "+QString::number(pendingBrightness)+"%");};
    std::function<void()> launch,startEngine;
    std::function<void(QJsonObject)> sendCommand;
    auto fail=[&](QString message){
        engineTimeout.stop();engineBusy=false;queued=QJsonObject();
        setStatus("Falha nos efeitos: "+message+". Use Reaplicar.");
        if(checkSync) app.exit(1);
    };
    startEngine=[&]{
        engineReady=false;engineBuffer.clear();engineTimeout.start();
        setStatus("Iniciando controle dos LEDs…");
        QString python=settings.value("rgbPython",base+"/runtime/bin/python").toString();
        QString script=settings.value("rgbEngine",base+"/src/rgb_engine.py").toString();
        engine.start(python,{script});
    };
    sendCommand=[&](QJsonObject command){
        if(!engineReady||engineBusy) {queued=command;return;}
        running=command;engineBusy=true;engineTimeout.start();
        setStatus("Aplicando efeito em todos os LEDs…");
        engine.write(QJsonDocument(command).toJson(QJsonDocument::Compact)+"\n");
    };
    launch=[&]{
        QJsonObject keyboard{{"enabled",m87Enabled},{"driver",settings.value("m87Driver").toString()},
            {"connection",settings.value("m87Connection","auto").toString()},{"independent",m87Independent->isChecked()},
            {"mode",m87Mode->currentIndex()},{"speed",m87Speed->value()},{"direction",m87Direction->currentIndex()},{"rainbow",m87Rainbow->isChecked()}};
        QJsonObject command{{"id",++nextId},{"effect",globalEffect->currentData().toString()},{"color",pending.name()},
            {"brightness",pendingBrightness},{"speed",globalSpeed->value()},{"keyboard",keyboard}};
        if(engine.state()==QProcess::NotRunning) {queued=command;startEngine();return;}
        sendCommand(command);
    };
    QObject::connect(&engineTimeout,&QTimer::timeout,[&]{engineReady=false;engine.kill();fail("o controle não respondeu no prazo");});
    QObject::connect(&engine,&QProcess::errorOccurred,[&](QProcess::ProcessError error){if(error==QProcess::FailedToStart) fail("motor RGB não instalado ou indisponível");});
    QObject::connect(&engine,&QProcess::readyReadStandardOutput,[&]{
        engineBuffer+=engine.readAllStandardOutput();
        while(engineBuffer.contains('\n')) {
            int index=engineBuffer.indexOf('\n');QByteArray line=engineBuffer.left(index);engineBuffer.remove(0,index+1);
            auto result=QJsonDocument::fromJson(line).object();QString event=result.value("event").toString();
            if(event=="ready") {
                engineReady=true;engineTimeout.stop();
                if(!queued.isEmpty()) {auto command=queued;queued=QJsonObject();sendCommand(command);}
            } else if(event=="applied" && result.value("id").toInteger()==running.value("id").toInteger()) {
                engineBusy=false;engineTimeout.stop();
                current=QColor(running.value("color").toString());
                settings.setValue("color",current.name());settings.setValue("brightness",running.value("brightness").toInt());
                settings.setValue("globalEffect",running.value("effect").toString());settings.setValue("globalSpeed",running.value("speed").toInt());
                auto keyboard=running.value("keyboard").toObject();
                settings.setValue("m87Independent",keyboard.value("independent").toBool());settings.setValue("m87Mode",keyboard.value("mode").toInt());
                settings.setValue("m87Speed",keyboard.value("speed").toInt());settings.setValue("m87Direction",keyboard.value("direction").toInt());settings.setValue("m87Rainbow",keyboard.value("rainbow").toBool());settings.sync();
                tray.setIcon(swatch(current));
                QStringList names;for(auto name:result.value("devices").toArray()) names<<name.toString();
                if(keyboard.value("enabled").toBool()) names<<"ATTACK SHARK M87";
                devices->setText("Controladores atendidos:\n"+names.join("\n"));
                setStatus(QString("Efeito enviado a %1 controladores. Confira as luzes.").arg(names.size()));
                if(checkSync) {app.exit(0);return;}
                if(!queued.isEmpty()) {auto command=queued;queued=QJsonObject();sendCommand(command);}
            } else if(event=="error"||event=="fatal") {fail(result.value("message").toString());}
        }
    });
    QObject::connect(&engine,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),[&](int,QProcess::ExitStatus){engineReady=false;if(!shuttingDown) fail("motor RGB foi encerrado");});

    QProcess waterQuery,waterControl;
    QTimer waterTimer;waterTimer.setInterval(2000);
    bool waterActive=false;
    std::function<void()> refreshWater;
    refreshWater=[&]{if(waterQuery.state()==QProcess::NotRunning)waterQuery.start("/usr/bin/systemctl",{"--user","show","rise-mode-display.service","--property=LoadState","--property=ActiveState"});};
    QObject::connect(&waterQuery,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),[&](int,QProcess::ExitStatus){
        auto output=waterQuery.readAllStandardOutput();bool installed=output.contains("LoadState=loaded");waterActive=output.contains("ActiveState=active");
        waterToggle->setEnabled(installed&&waterControl.state()==QProcess::NotRunning);waterToggle->setText(waterActive?"Parar telinha":"Iniciar telinha");
        QFile file(QDir::homePath()+"/.local/state/rise-mode-aura-ice/last-state.json");QJsonObject snapshot;if(file.open(QIODevice::ReadOnly)) snapshot=QJsonDocument::fromJson(file.readAll()).object();
        auto timestamp=QDateTime::fromString(snapshot.value("timestamp").toString(),Qt::ISODateWithMs);
        qint64 age=timestamp.isValid()?timestamp.msecsTo(QDateTime::currentDateTimeUtc()):-1;
        bool fresh=age>=0&&age<=8000;
        if(!installed||!waterActive||!fresh) {
            waterStats->setText(!installed?"Monitor da telinha não instalado.":!waterActive?"Telinha pausada.":"Aguardando dados atuais da telinha…");
            waterState->setText(!installed?"Instale o monitor Rise Mode para habilitar a telinha.":!waterActive?"Monitor parado. Você pode iniciar por aqui.":"Os dados não foram atualizados nos últimos segundos.");return;
        }
        auto metrics=snapshot.value("metrics").toObject();
        auto metric=[&](QString key,QString unit){auto value=metrics.value(key);return value.isDouble()?QString::number(value.toInt())+unit:QString("—");};
        waterStats->setText("CPU "+metric("cpu_temperature"," °C")+" · GPU "+metric("gpu_temperature"," °C")+"\nUso CPU "+metric("cpu_usage","%")+" · RAM "+metric("memory_usage","%"));
        waterState->setText(snapshot.value("status").toString()=="report_written"?"Telinha recebendo dados a cada 2 segundos.":"Falha na telinha: "+snapshot.value("error").toString("confira o monitor Rise Mode."));
    });
    QObject::connect(waterSource,qOverload<int>(&QComboBox::activated),[&](int){
        QDir().mkpath(QFileInfo(waterConfig).absolutePath());QSaveFile file(waterConfig);
        QByteArray data=QJsonDocument(QJsonObject{{"temperature_source",waterSource->currentData().toString()}}).toJson();
        if(!file.open(QIODevice::WriteOnly)||file.write(data)!=data.size()||!file.commit()){waterSource->setCurrentIndex(savedWaterIndex);waterSourceError->setText("Não foi possível salvar a escolha da telinha. Tente novamente.");waterSourceError->show();return;}
        savedWaterIndex=waterSource->currentIndex();waterSourceError->hide();
        waterState->setText("Escolha salva. Aguarde a próxima atualização da telinha.");
    });
    QObject::connect(waterToggle,&QPushButton::clicked,[&]{waterToggle->setEnabled(false);waterControl.start("/usr/bin/systemctl",{"--user",waterActive?"stop":"start","rise-mode-display.service"});});
    QObject::connect(&waterControl,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),[&](int code,QProcess::ExitStatus){if(code!=0)waterState->setText("Não foi possível alterar a telinha. Tente novamente.");refreshWater();});
    QObject::connect(&waterTimer,&QTimer::timeout,refreshWater);
    if(!checkSync) {waterTimer.start();refreshWater();}
    auto choose=[&](QColor color){if(!color.isValid())return;pending=color;
        {QSignalBlocker block(globalEffect);if(color==QColor(Qt::black))globalEffect->setCurrentIndex(globalEffect->findData("off"));else if(globalEffect->currentData().toString()=="off")globalEffect->setCurrentIndex(0);}
        updatePreview();launch();};
    const QList<QPair<QString,QString>> colors={{"Azul","#0000ff"},{"Ciano","#00c8ff"},{"Verde","#00ff00"},{"Amarelo","#ffcc00"},{"Vermelho","#ff0000"},{"Roxo","#8000ff"},{"Branco","#ffffff"},{"Apagar","#000000"}};
    int index=0;
    for(auto preset:colors) {
        QColor color(preset.second);
        auto action=menu.addAction(swatch(color),preset.first); QObject::connect(action,&QAction::triggered,[&,color]{choose(color);});
        auto button=new QPushButton(swatch(color),QString()); button->setIconSize(QSize(26,26)); button->setToolTip(preset.first); button->setAccessibleName(preset.first); button->setMinimumHeight(44); presets->addWidget(button,index/4,index%4); ++index;
        QObject::connect(button,&QPushButton::clicked,[&,color]{choose(color);});
    }
    auto selectColor=[&]{ QColor color=QColorDialog::getColor(pending,nullptr,"Escolher cor dos LEDs"); if(color.isValid()) choose(color); };
    QObject::connect(custom,&QPushButton::clicked,selectColor); QObject::connect(preview,&QPushButton::clicked,selectColor);
    QObject::connect(brightness,&QSlider::valueChanged,[&](int value){pendingBrightness=value; brightnessLabel->setText(QString("Brilho · %1%").arg(value)); updatePreview();});
    QObject::connect(brightness,&QSlider::sliderReleased,launch);
    QTimer debounce; debounce.setSingleShot(true); debounce.setInterval(500);
    QObject::connect(brightness,&QSlider::valueChanged,[&]{debounce.start();}); QObject::connect(&debounce,&QTimer::timeout,launch);
    auto updateEffectControls=[&]{
        int mode=m87Mode->currentIndex();
        m87Options->setEnabled(m87Independent->isChecked());m87Speed->setEnabled(mode>1);m87Rainbow->setEnabled(mode>0);
        m87Direction->setEnabled(mode==10||mode==11||mode==12||mode==16||mode==18);
    };
    QObject::connect(globalEffect,qOverload<int>(&QComboBox::currentIndexChanged),[&]{launch();});
    QObject::connect(globalSpeed,&QSlider::valueChanged,[&]{debounce.start();});
    QObject::connect(m87Independent,&QCheckBox::toggled,[&]{updateEffectControls();launch();});
    QObject::connect(m87Mode,qOverload<int>(&QComboBox::currentIndexChanged),[&]{updateEffectControls();launch();});
    QObject::connect(m87Speed,qOverload<int>(&QSpinBox::valueChanged),[&]{debounce.start();});
    QObject::connect(m87Direction,qOverload<int>(&QComboBox::currentIndexChanged),[&]{launch();});
    QObject::connect(m87Rainbow,&QCheckBox::toggled,[&]{launch();});
    updateEffectControls();
    QObject::connect(retry,&QPushButton::clicked,launch);
    menu.addSeparator();
    auto pick=menu.addAction("Escolher cor…"); QObject::connect(pick,&QAction::triggered,selectColor);
    auto open=menu.addAction("Abrir controle…"); auto show=[&]{
        if(window.isVisible()) { window.hide(); return; }
        QPoint anchor=QCursor::pos();
        QRect icon=tray.geometry();
        if(icon.isValid()) anchor=QPoint(icon.center().x(),icon.bottom());
        // Use the actual Plasma tray bounds, also when activation comes from D-Bus.
        QProcess panelBounds;
        panelBounds.start("/usr/bin/qdbus", {"org.kde.plasmashell", "/PlasmaShell", "org.kde.PlasmaShell.evaluateScript",
            "var p=panels().filter(p=>p.location==='top')[0]; if(p){var t=p.widgets().filter(w=>w.type==='org.kde.plasma.systemtray')[0]; if(t) print(JSON.stringify(t.geometry));}"});
        if(panelBounds.waitForFinished(1000)) {
            auto rect=QJsonDocument::fromJson(panelBounds.readAllStandardOutput()).object();
            if(rect.contains("x")) {
                QRect trayBounds(rect["x"].toInt(),rect["y"].toInt(),rect["width"].toInt(),rect["height"].toInt());
                int anchorX=trayBounds.contains(anchor)?anchor.x():trayBounds.left()+16;
                anchor=QPoint(anchorX,trayBounds.bottom());
            }
        }
        auto screen=QGuiApplication::screenAt(anchor); if(!screen) screen=QGuiApplication::primaryScreen();
        QRect available=screen->availableGeometry();
        window.setMaximumHeight(qMax(100,available.height()-16));
        window.adjustSize();window.resize(window.width(),qMin(body->sizeHint().height()+2,available.height()-16));
        int x=qBound(available.left()+8,anchor.x()-window.width()/2,available.right()-window.width()-8);
        int y=qMax(available.top()+8,anchor.y()+10);
        y=qMin(y,available.bottom()-window.height()-8);
        window.move(x,y); window.show(); window.raise(); window.activateWindow();
    }; QObject::connect(open,&QAction::triggered,show);
    auto diagnostic=menu.addAction("Ver diagnóstico"); QObject::connect(diagnostic,&QAction::triggered,[&]{ QDesktopServices::openUrl(QUrl::fromLocalFile(base+"/effects-engine.log")); });
    menu.addSeparator(); auto quit=menu.addAction("Sair do controle"); QObject::connect(quit,&QAction::triggered,&app,&QApplication::quit);
    tray.setContextMenu(&menu); tray.show();
    QObject::connect(&tray,&QSystemTrayIcon::activated,[&](QSystemTrayIcon::ActivationReason reason){if(reason==QSystemTrayIcon::Trigger) show();});
    QObject::connect(&server,&QLocalServer::newConnection,[&]{auto socket=server.nextPendingConnection(); if(socket){socket->deleteLater();show();}});
    updatePreview();
    if(capture) {
        details->setChecked(true);window.adjustSize();window.resize(window.width(),qMin(body->sizeHint().height()+2,window.screen()->availableGeometry().height()-16));window.show();
        QTimer::singleShot(200,[&]{window.grab().save(app.arguments().at(captureArg+1));app.quit();});
    } else QTimer::singleShot(0,launch);
    if(app.arguments().contains("--show")) show();
    QObject::connect(&app,&QApplication::aboutToQuit,[&]{shuttingDown=true;waterTimer.stop();
        if(waterQuery.state()!=QProcess::NotRunning){waterQuery.kill();waterQuery.waitForFinished(1000);}
        if(waterControl.state()!=QProcess::NotRunning){waterControl.kill();waterControl.waitForFinished(1000);}
        if(engine.state()!=QProcess::NotRunning){engine.closeWriteChannel();engine.terminate();if(!engine.waitForFinished(4000)){engine.kill();engine.waitForFinished(1000);}}
    });
    return app.exec();
}
