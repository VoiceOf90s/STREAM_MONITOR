#ifndef ADD_STREAMER_DIALOG_H
#define ADD_STREAMER_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>


// Диалог для добавления нового стримера
class AddStreamerDialog : public QDialog {
  Q_OBJECT

private:
  QLineEdit* nameInput;
  QSpinBox* priorityInput;
  QPushButton* okButton;
  QPushButton* cancelButton;
  QLabel* warningLabel;
  
  void SetupUI();
  void ValidateInput();


private slots:
  void OnNameChanged(const QString& text);
  void OnOkClicked();


public:
  explicit AddStreamerDialog(int maxPriority, QWidget* parent = nullptr);
  
  QString GetStreamerName() const;
  int GetPriority() const;
};

#endif // ADD_STREAMER_DIALOG_H