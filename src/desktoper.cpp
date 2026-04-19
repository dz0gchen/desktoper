#include "desktoper.h"
#include <QDebug>

App::App(QWidget *parent) : QWidget(parent) {
  setWindowTitle("Desktoper");
  setMinimumSize(300, 400);

  m_searchEdit = new QLineEdit(this);
  m_searchEdit->setPlaceholderText("Input app...");
  m_searchEdit->setFocus();

  m_listWidget = new QListWidget(this);

  m_workspaceGroup = new QGroupBox(this);
  m_workspaceLayout = new QVBoxLayout(m_workspaceGroup);

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->addWidget(m_searchEdit);
  mainLayout->addWidget(m_workspaceGroup);
  mainLayout->addWidget(m_listWidget);
  setLayout(mainLayout);

  connect(m_searchEdit, &QLineEdit::textChanged, this,
          &App::onSearchTextChanged);
  connect(m_listWidget, &QListWidget::itemDoubleClicked, this,
          &App::onItemDoubleClicked);

  loadApplications();
  updateWorkspaceCheckboxes();
}

App::~App() {}

void App::loadApplications() {
  QStringList paths = {"/usr/share/applications/",
                       QDir::homePath() + "/.local/share/applications/"};

  m_apps.clear();

  for (const QString &path : paths) {
    QDir dir(path);
    if (!dir.exists())
      continue;

    QStringList files =
        dir.entryList(QStringList() << "*.desktop", QDir::Files);

    for (const QString &file : files) {
      QString fullPath = path + file;
      QSettings desktop(fullPath, QSettings::IniFormat);
      desktop.beginGroup("Desktop Entry");

      if (desktop.value("NoDisplay").toBool() ||
          desktop.value("Hidden").toBool()) {
        continue;
      }

      QString name = desktop.value("Name").toString();
      QString exec = desktop.value("Exec").toString();

      if (name.isEmpty() || exec.isEmpty())
        continue;

      exec.remove(" %f");
      exec.remove(" %F");
      exec.remove(" %u");
      exec.remove(" %U");
      exec = exec.trimmed();

      m_apps.append({name, exec});
    }
  }

  for (int i = 0; i < m_apps.size() - 1; ++i) {
    for (int j = i + 1; j < m_apps.size(); ++j) {
      if (m_apps[i].first > m_apps[j].first) {
        qSwap(m_apps[i], m_apps[j]);
      }
    }
  }
}

int App::getWorkspaceCount() {
  QProcess process;
  process.start("gsettings", QStringList() << "get"
                                           << "org.gnome.desktop.wm.preferences"
                                           << "num-workspaces");
  process.waitForFinished();

  QString output = process.readAllStandardOutput().trimmed();

  if (!output.isEmpty()) {
    bool ok;
    int count = output.toInt(&ok);
    if (ok) {
      return count;
    }
  }

  return 1;
}

void App::createWorkspaceCheckboxes(int count) {
  for (int i = 0; i < count; ++i) {
    QRadioButton *radio = new QRadioButton(QString("Space %1").arg(i + 1));

    if (i == 0) {
      radio->setChecked(true);
    }

    m_workspaceLayout->addWidget(radio);
    m_workspaceButtons.append(radio);
  }
}

void App::updateWorkspaceCheckboxes() {
  int workspaceCount = getWorkspaceCount();
  createWorkspaceCheckboxes(workspaceCount);
}

void App::onSearchTextChanged(const QString &text) {
  m_listWidget->clear();

  if (text.isEmpty()) {
    return;
  }

  QString lowerText = text.toLower();

  for (const auto &app : m_apps) {
    if (app.first.toLower().contains(lowerText)) {
      QListWidgetItem *item = new QListWidgetItem(app.first);
      item->setData(Qt::UserRole, app.second);
      m_listWidget->addItem(item);
    }
  }
}

void App::onItemDoubleClicked(QListWidgetItem *item) {
  int workspaceIndex = getSelectedWorkspace();

  QString command_args = item->data(Qt::UserRole).toString();
  if (command_args.isEmpty())
    return;

  QStringList cmd_args = command_args.split(" ");
  QStringList args = cmd_args.mid(1);
  QString cmd = cmd_args[0];

  qDebug() << "cmd:" << cmd;
  qDebug() << "args:" << args;

  QProcess::startDetached(cmd, args, QString(), nullptr);

  bool usr_lib = cmd.startsWith("/usr/lib/");
  if (usr_lib) {
    cmd = cmd.section("/", -1);
  }

  QTimer::singleShot(3000, this, [cmd, workspaceIndex]() {
    QProcess wmctrl;
    wmctrl.start("wmctrl", QStringList() << "-lpx");
    wmctrl.waitForFinished();

    QString output = wmctrl.readAllStandardOutput();
    QStringList lines = output.split("\n", Qt::SkipEmptyParts);

    struct WindowInfo {
      QString windowId;
      qint64 pid;
      QString windowCls;
    };

    WindowInfo info;

    for (const QString &line : lines) {
      QStringList parts = line.split(" ", Qt::SkipEmptyParts);

      info.windowId = parts[0];
      info.pid = parts[2].toLongLong();
      info.windowCls = parts[3];

      if (info.windowCls.contains(cmd)) {
        qDebug() << "----------match win cls and cmd -------------";
        qDebug() << "window_id:" << info.windowId;
        qDebug() << "window_cls:" << info.windowCls;
        qDebug() << "cmd:" << cmd;
        qDebug() << "---------------------------------------------";
        QProcess moveProcess;
        moveProcess.start("wmctrl", QStringList()
                                        << "-i"
                                        << "-r" << info.windowId << "-t"
                                        << QString::number(workspaceIndex));
        moveProcess.waitForFinished();
      }
    }
  });
}

int App::getSelectedWorkspace() {
  for (int i = 0; i < m_workspaceButtons.size(); ++i) {
    if (m_workspaceButtons[i]->isChecked()) {
      return i;
    }
  }

  return 0;
}
