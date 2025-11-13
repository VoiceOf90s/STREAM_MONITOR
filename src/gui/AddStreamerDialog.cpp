#include "AddStreamerDialog.h"
#include "StringUtils.h"
#include <QFormLayout>
#include <QDialogButtonBox>


AddStreamerDialog::AddStreamerDialog(int maxPriority, QWidget* parent)
  : QDialog(parent) {
  
  setWindowTitle("Add New Streamer");
  setMinimumWidth(400);
  
  SetupUI();
  
  priorityInput->setMaximum(maxPriority);
  priorityInput->setValue(maxPriority);
  
  connect(nameInput, &QLineEdit::textChanged, this, &AddStreamerDialog::OnNameChanged);
  connect(okButton, &QPushButton::clicked, this, &AddStreamerDialog::OnOkClicked);
  connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}


void AddStreamerDialog::SetupUI() {
  QVBoxLayout* mainLayout = new QVBoxLayout(this);
  
  QFormLayout* formLayout = new QFormLayout();
  
  nameInput = new QLineEdit(this);
  nameInput->setPlaceholderText("e.g. shroud, pokimane, xqc");
  nameInput->setMaxLength(25);
  
  priorityInput = new QSpinBox(this);
  priorityInput->setMinimum(0);
  priorityInput->setToolTip("0 = highest priority (checked first)");
  
  formLayout->addRow("Streamer Name:", nameInput);
  formLayout->addRow("Priority:", priorityInput);
  
  mainLayout->addLayout(formLayout);
  
  warningLabel = new QLabel(this);
  warningLabel->setStyleSheet("QLabel { color: #f44336; font-weight: bold; }");
  warningLabel->setWordWrap(true);
  warningLabel->hide();
  
  mainLayout->addWidget(warningLabel);
  
  QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
  
  okButton = buttonBox->addButton(QDialogButtonBox::Ok);
  okButton->setEnabled(false);
  
  cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);
  
  mainLayout->addWidget(buttonBox);
  
  setLayout(mainLayout);
}


void AddStreamerDialog::ValidateInput() {
  QString name = nameInput->text().trimmed();
  
  if (name.isEmpty()) {
    okButton->setEnabled(false);
    warningLabel->hide();
    return;
  }
  
  if (!StringUtils::IsValidStreamerName(name.toStdString())) {
    okButton->setEnabled(false);
    warningLabel->setText("⚠️ Invalid name: only letters, numbers, and underscore allowed (1-25 characters)");
    warningLabel->show();
    return;
  }
  
  okButton->setEnabled(true);
  warningLabel->hide();
}


void AddStreamerDialog::OnNameChanged(const QString& text) {
  ValidateInput();
}


void AddStreamerDialog::OnOkClicked() {
  QString name = nameInput->text().trimmed();
  
  if (name.isEmpty() || !StringUtils::IsValidStreamerName(name.toStdString())) {
    warningLabel->setText("⚠️ Please enter a valid streamer name");
    warningLabel->show();
    return;
  }
  
  accept();
}


QString AddStreamerDialog::GetStreamerName() const {
  return nameInput->text().trimmed();
}


int AddStreamerDialog::GetPriority() const {
  return priorityInput->value();
}