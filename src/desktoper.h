#ifndef APP_H
#define APP_H

#include <QDir>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QProcess>
#include <QRadioButton>
#include <QSettings>
#include <QString>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

class App : public QWidget {
  Q_OBJECT

 public:
  App(QWidget *parent = nullptr);
  ~App();

 private slots:
  void onSearchTextChanged(const QString &text);
  void onItemDoubleClicked(QListWidgetItem *item);
  void updateWorkspaceCheckboxes();

 private:
  void loadApplications();
  int getWorkspaceCount();
  int getSelectedWorkspace();
  void createWorkspaceCheckboxes(int count);

  QLineEdit *m_searchEdit;
  QListWidget *m_listWidget;
  QGroupBox *m_workspaceGroup;
  QVBoxLayout *m_workspaceLayout;
  QVector<QRadioButton *> m_workspaceButtons;
  QList<QPair<QString, QString>> m_apps;
};

#endif  // APP_H
