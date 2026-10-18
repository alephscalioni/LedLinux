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
    window.setObjectName("ledPopover"); window.setWindowTitle("Cores dos LEDs"); window.setFixedWidth(344);
    window.setStyleSheet("QFrame#ledPopover {background: #222732; border: 1px solid #50596b; border-radius: 16px;} QLabel {color: #f3f5fa;} QPushButton {padding: 8px 6px; border-radius: 8px; background: #363e4e; color: #f3f5fa; border: none;} QPushButton:hover {background: #46536a;} QPushButton:focus {border: 1px solid #8bbcff;} QCheckBox {color: #d6dce7;}");
    auto layout=new QVBoxLayout(&window); layout->setContentsMargins(20,20,20,20); layout->setSpacing(12);
    auto heading=new QLabel("Cores dos LEDs"); QFont f=heading->font(); f.setPixelSize(16); f.setWeight(QFont::DemiBold); f.setLetterSpacing(QFont::AbsoluteSpacing, 0.1); heading->setFont(f); layout->addWidget(heading);
    auto description=new QLabel("Iluminação sincronizada"); description->setWordWrap(true); layout->addWidget(description);
    auto preview=new QPushButton; preview->setMinimumHeight(38); layout->addWidget(preview);
    auto presets=new QGridLayout; presets->setSpacing(8); layout->addLayout(presets);
    auto custom=new QPushButton("Escolher outra cor…"); layout->addWidget(custom);
    auto brightnessLabel=new QLabel("Brilho"); layout->addWidget(brightnessLabel);
    auto brightness=new QSlider(Qt::Horizontal); brightness->setRange(0,100); brightness->setValue(settings.value("brightness",100).toInt()); brightness->setAccessibleName("Brilho dos LEDs"); layout->addWidget(brightness);
    auto effectRow=new QWidget; auto effectLayout=new QFormLayout(effectRow); effectLayout->setContentsMargins(0,0,0,0);
    auto m87Mode=new QComboBox;
    m87Mode->addItems({"Desligado", "Estático", "Acender ao tocar", "Apagar ao tocar", "Cintilação", "Queda", "Colorido", "Respiração", "Espectro", "Expansão", "Rolagem horizontal", "Rolagem vertical", "Rotação", "Explosão", "Lançamento", "Ondas", "Fluxo", "Pulsação", "Inclinação", "Travessia"});
    m87Mode->setCurrentIndex(qBound(0,settings.value("m87Mode",1).toInt(),19));
    m87Mode->setAccessibleName("Efeito RGB do teclado M87");
    effectLayout->addRow("Efeito do M87",m87Mode); layout->addWidget(effectRow);effectRow->setVisible(m87Enabled);
    auto status=new QLabel; status->setWordWrap(true); status->setTextInteractionFlags(Qt::TextSelectableByMouse); layout->addWidget(status);
    auto details=new QToolButton; details->setText("Opções e dispositivos"); details->setCheckable(true); details->setToolButtonStyle(Qt::ToolButtonTextBesideIcon); details->setArrowType(Qt::RightArrow); layout->addWidget(details);
    auto advanced=new QWidget; auto advancedLayout=new QVBoxLayout(advanced); advancedLayout->setContentsMargins(0,0,0,0); advancedLayout->setSpacing(10); advanced->hide(); layout->addWidget(advanced);
    auto devices=new QLabel; devices->setWordWrap(true); advancedLayout->addWidget(devices);
    QObject::connect(details,&QToolButton::toggled,[&](bool checked){ advanced->setVisible(checked); details->setArrowType(checked?Qt::DownArrow:Qt::RightArrow); window.adjustSize(); });
    auto m87Options=new QWidget; auto m87Form=new QFormLayout(m87Options);m87Form->setContentsMargins(0,0,0,0);
    auto m87Speed=new QSpinBox;m87Speed->setRange(0,5);m87Speed->setValue(qBound(0,settings.value("m87Speed",3).toInt(),5));
    m87Form->addRow("Velocidade M87",m87Speed);
    auto m87Direction=new QComboBox;m87Direction->addItems({"Normal","Inverso"});m87Direction->setCurrentIndex(qBound(0,settings.value("m87Direction",0).toInt(),1));
    m87Form->addRow("Sentido M87",m87Direction);
    auto m87Rainbow=new QCheckBox("Multicolorido no M87");m87Rainbow->setChecked(settings.value("m87Rainbow",false).toBool());m87Form->addRow(m87Rainbow);
    auto m87Note=new QLabel("Efeito apenas no teclado. Lateral e setinha continuam pelo controle físico.");m87Note->setWordWrap(true);m87Note->setMinimumHeight(m87Note->fontMetrics().boundingRect(QRect(0,0,290,1000),Qt::TextWordWrap,m87Note->text()).height()+4);m87Form->addRow(m87Note);
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
    QProcess process; process.setProcessChannelMode(QProcess::MergedChannels);
    QTimer timeout; timeout.setSingleShot(true); timeout.setInterval(45000);
    QObject::connect(&timeout,&QTimer::timeout,[&]{ process.kill(); });
    QColor pending=current, running=current; int pendingBrightness=brightness->value(), runningBrightness=pendingBrightness;
    bool queued=false, busy=false;
    QProcess keyboard; keyboard.setProcessChannelMode(QProcess::MergedChannels);
    QTimer keyboardTimeout; keyboardTimeout.setSingleShot(true); keyboardTimeout.setInterval(10000);
    QObject::connect(&keyboardTimeout,&QTimer::timeout,[&]{ keyboard.kill(); });
    QStringList confirmedNames;
    bool computerFailed=false;
    int runningMode=m87Mode->currentIndex(), runningSpeed=m87Speed->value(), runningDirection=m87Direction->currentIndex();
    bool runningRainbow=m87Rainbow->isChecked();
    const QString m87Driver=settings.value("m87Driver").toString();
    const QString m87Connection=settings.value("m87Connection", "auto").toString();
    auto setStatus=[&](QString message){status->setText(message); statusAction->setText(message); tray.setToolTip("Cores dos LEDs\n"+message);};
    std::function<void()> launch;
    auto updatePreview=[&]{ preview->setIcon(swatch(pending)); preview->setText(pending.name().toUpper()+" · "+QString::number(pendingBrightness)+"%"); };
    std::function<void(QString)> complete;
    complete=[&](QString keyboardError){
        busy=false;
        QStringList listed=confirmedNames;
        if(m87Enabled && keyboardError.isEmpty()) listed<<"ATTACK SHARK M87 (resposta recebida)";
        devices->setText(listed.isEmpty()?"Nenhum dispositivo RGB confirmado.":"Dispositivos detectados:\n"+listed.join("\n"));
        if(computerFailed || !keyboardError.isEmpty()) {
            QString message=computerFailed?"OpenRGB: falha ao confirmar os LEDs do computador.":"LEDs do computador confirmados.";
            if(!keyboardError.isEmpty()) message+=" M87: "+keyboardError;
            setStatus(message);
            tray.showMessage("Cores dos LEDs",message,QSystemTrayIcon::Warning);
        } else {
            if(m87Enabled) {
                settings.setValue("m87Mode",runningMode);settings.setValue("m87Speed",runningSpeed);
                settings.setValue("m87Direction",runningDirection);settings.setValue("m87Rainbow",runningRainbow);
            }
            current=running; settings.setValue("color",current.name()); settings.setValue("brightness",runningBrightness); settings.sync();
            tray.setIcon(swatch(QColor(current.red()*runningBrightness/100,current.green()*runningBrightness/100,current.blue()*runningBrightness/100)));
            setStatus(QString("%1 dispositivos com resposta confirmada; confira as luzes").arg(listed.size()));
        }
        if(checkSync) { app.exit(computerFailed || !keyboardError.isEmpty() ? 1 : 0); return; }
        if(queued) QTimer::singleShot(0,launch);
    };
    QObject::connect(&keyboard,&QProcess::errorOccurred,[&](QProcess::ProcessError error){
        if(error==QProcess::FailedToStart) { keyboardTimeout.stop(); complete("não foi possível abrir o controlador Python."); }
    });
    QObject::connect(&keyboard,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),[&](int code,QProcess::ExitStatus exit){
        keyboardTimeout.stop();
        const auto output=keyboard.readAll();
        QFile log(base+"/last-m87-run.log"); if(log.open(QIODevice::WriteOnly)) log.write(output);
        auto result=QJsonDocument::fromJson(output).object();
        bool acknowledged=result.value("device_acknowledged").toBool() || result.value("device_response_matched").toBool();
        complete(code==0 && exit==QProcess::NormalExit && acknowledged ? QString() : QString::fromUtf8(output).trimmed().left(500).prepend("falha na comunicação. "));
    });
    launch=[&]{
        if(busy || process.state()!=QProcess::NotRunning) { queued=true; return; }
        busy=true; queued=false; running=pending; runningBrightness=pendingBrightness;
        runningMode=m87Mode->currentIndex();runningSpeed=m87Speed->value();runningDirection=m87Direction->currentIndex();runningRainbow=m87Rainbow->isChecked();
        QColor output(running.red()*runningBrightness/100,running.green()*runningBrightness/100,running.blue()*runningBrightness/100);
        QStringList args{"--noautoconnect"};
        for(auto name: {"Corsair Vengeance","Gigabyte GeForce","G502","ASUS TUF","Nollie 8CH"})
            args<<"--device"<<name<<"--mode"<<((QString(name)=="G502"||QString(name)=="ASUS TUF")?"Static":"Direct")<<"--color"<<output.name().mid(1);
        args<<"--save-profile"<<"Aleph-LastApplied";
        QFile::remove(QDir::homePath()+"/.config/OpenRGB/profiles/Aleph-LastApplied.json");
        setStatus("Aplicando cor…");
        process.start(base+"/vendor/squashfs-root/AppRun",args); timeout.start();
    };
    QObject::connect(&process,&QProcess::errorOccurred,[&](QProcess::ProcessError error){
        if(error==QProcess::FailedToStart) { timeout.stop(); computerFailed=true; confirmedNames.clear(); complete(m87Enabled?"não enviado porque o OpenRGB não iniciou.":QString()); }
    });
    QObject::connect(&process,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),[&](int code,QProcess::ExitStatus exitStatus){
        timeout.stop(); QString output=QString::fromUtf8(process.readAll());
        QFile log(base+"/last-run.log"); if(log.open(QIODevice::WriteOnly)) log.write(output.toUtf8());
        QStringList names;
        bool failed=code!=0||exitStatus!=QProcess::NormalExit;
        QFile profile(QDir::homePath()+"/.config/OpenRGB/profiles/Aleph-LastApplied.json");
        if(!profile.open(QIODevice::ReadOnly)) failed=true;
        const auto controllers=QJsonDocument::fromJson(profile.readAll()).object().value("controllers").toArray();
        const int expectedColor=(running.red()*runningBrightness/100)|((running.green()*runningBrightness/100)<<8)|((running.blue()*runningBrightness/100)<<16);
        for(auto value:controllers) {
            auto controller=value.toObject(); names<<controller.value("name").toString();
            auto modes=controller.value("modes").toArray();
            int active=controller.value("active_mode").toInt(-1);
            if(active<0||active>=modes.size()) { failed=true; continue; }
            auto mode=modes[active].toObject();
            auto colors=(mode.value("color_mode").toInt()==2?mode.value("colors"):controller.value("colors")).toArray();
            if(colors.isEmpty()) failed=true;
            for(auto color:colors) if(color.toInt(-1)!=expectedColor) failed=true;
        }
        for(auto expected:{"Gigabyte GeForce RTX 3060 Gaming OC","G502 LIGHTSPEED Wireless Gaming Mouse","ASUS TUF GAMING B550M-PLUS","Nollie 8CH"})
            if(!names.contains(expected)) failed=true;
        if(names.count("Corsair Vengeance RGB Pro DDR4")!=2) failed=true;
        QRegularExpression errors("error|failed|failure|cannot|could not|out of range|not found|wrong number",QRegularExpression::CaseInsensitiveOption);
        if(errors.match(output).hasMatch()) failed=true;
        confirmedNames=names; computerFailed=failed;
        if(!m87Enabled) { complete(QString()); return; }
        if(!QFileInfo(m87Driver).isFile()) { complete("controlador não instalado; confira m87Driver nas configurações."); return; }
        setStatus("Aplicando efeito ao M87…");
        QStringList keyboardArgs{m87Driver, "--color", running.name(), "--brightness", QString::number(runningBrightness), "--connection", m87Connection,
                                 "--mode",QString::number(runningMode),"--speed",QString::number(runningSpeed),"--direction",QString::number(runningDirection),"--apply"};
        if(runningRainbow) keyboardArgs<<"--rainbow";
        keyboard.start("/usr/bin/python3", keyboardArgs);
        keyboardTimeout.start();
    });
    auto choose=[&](QColor color){ if(!color.isValid()) return; pending=color; updatePreview(); launch(); };
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
        m87Speed->setEnabled(mode>1);m87Rainbow->setEnabled(mode>0);
        m87Direction->setEnabled(mode==10||mode==11||mode==12||mode==16||mode==18);
    };
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
        window.adjustSize();
        int x=qBound(available.left()+8,anchor.x()-window.width()/2,available.right()-window.width()-8);
        int y=qMax(available.top()+8,anchor.y()+10);
        y=qMin(y,available.bottom()-window.height()-8);
        window.move(x,y); window.show(); window.raise(); window.activateWindow();
    }; QObject::connect(open,&QAction::triggered,show);
    auto diagnostic=menu.addAction("Ver diagnóstico"); QObject::connect(diagnostic,&QAction::triggered,[&]{ QDesktopServices::openUrl(QUrl::fromLocalFile(base+"/last-run.log")); });
    menu.addSeparator(); auto quit=menu.addAction("Sair do controle"); QObject::connect(quit,&QAction::triggered,&app,&QApplication::quit);
    tray.setContextMenu(&menu); tray.show();
    QObject::connect(&tray,&QSystemTrayIcon::activated,[&](QSystemTrayIcon::ActivationReason reason){if(reason==QSystemTrayIcon::Trigger) show();});
    QObject::connect(&server,&QLocalServer::newConnection,[&]{auto socket=server.nextPendingConnection(); if(socket){socket->deleteLater();show();}});
    updatePreview();
    if(capture) {
        details->setChecked(true);window.adjustSize();window.show();
        QTimer::singleShot(200,[&]{window.grab().save(app.arguments().at(captureArg+1));app.quit();});
    } else QTimer::singleShot(1500,launch);
    if(app.arguments().contains("--show")) show();
    QObject::connect(&app,&QApplication::aboutToQuit,[&]{if(keyboard.state()!=QProcess::NotRunning){keyboard.kill();keyboard.waitForFinished(1000);}if(process.state()!=QProcess::NotRunning){process.terminate();if(!process.waitForFinished(2000)){process.kill();process.waitForFinished(1000);}}});
    return app.exec();
}
