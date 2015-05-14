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
    ~InProgressDialog();

    static InProgressDialog* create();

    void closeAndDeleteLater(){
        close();
        deleteLater();
    }


private:
    Ui::InProgressDialog *ui;
    explicit InProgressDialog(QWidget *parent = 0);

};

#endif // INPROGRESSDIALOG_H
