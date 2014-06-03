#ifndef INPROGRESSDIALOG_H
#define INPROGRESSDIALOG_H

#include <QDialog>

namespace Ui {
class InProgressDialog;
}

class InProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InProgressDialog(QWidget *parent = 0);
    ~InProgressDialog();

private:
    Ui::InProgressDialog *ui;
};

#endif // INPROGRESSDIALOG_H
