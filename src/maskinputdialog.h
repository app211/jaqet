#ifndef MASKINPUTDIALOG_H
#define MASKINPUTDIALOG_H

#include <QDialog>

class QLabel;
class QLineEdit;
class QDialogButtonBox;
class QRegExpValidator;


class MaskInputDialog : public QDialog
{

    Q_OBJECT
public:
    explicit MaskInputDialog(QWidget *parent = 0, Qt::WindowFlags=0);

    void setTitle(const QString &title);
    void setLabelText(const QString &label);
    void setText(const QString &text);
    void setInputMask(const QString &inputMask);

    QString getLabelText();
    QString getText();

    static QString getText(QWidget *parent, const QString &title, const QString &label, const QString &text, const QString &inputMask, bool *ok, Qt::WindowFlags flags=0);

signals:

private:
QLabel *label;
QLineEdit *text;
QDialogButtonBox *buttonBox;

};

#endif // MASKINPUTDIALOG_H
