#ifndef CLUSTERNODELISTMODEL_H
#define CLUSTERNODELISTMODEL_H


#include <QStandardItemModel>



class ClusterNodeListModel  : public QStandardItemModel
{
    Q_OBJECT

public:

    explicit ClusterNodeListModel(QObject *parent=nullptr);

    QString filename() const { return m_filename; }

    void clear();
    void load(const QString &filename=QString());
    void save(const QString &filename=QString());

private:
    void initialize();

    QString m_filename;
    void errorDialogue(QString msg);
};

#endif // CLUSTERNODELISTMODEL_H

