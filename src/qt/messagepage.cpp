#include "messagepage.h"
#include "ui_messagepage.h"

#include "sendmessagesdialog.h"
//#include "mrichtextedit.h"
#include "messagemodel.h"
#include "bitcoingui.h"
#include "csvmodelwriter.h"
#include "guiutil.h"

#include "walletmodel.h"
#include "addresstablemodel.h"
#include "wallet/wallet.h"

#include <QSortFilterProxyModel>
#include <QClipboard>
#include <QMessageBox>
#include <QMenu>
#include <QStyledItemDelegate>
#include <QAbstractTextDocumentLayout>
#include <QPainter>


#include <QToolBar>
#include <QMenu>

//#include <QDebug>

#define DECORATION_SIZE 64
#define NUM_ITEMS 3

QTreeWidgetItem *_selectedContactItem;
bool _updateConversationListRequired = true;

//#include <QAbstractItemDelegate>


class MessageViewDelegate : public QStyledItemDelegate
{
protected:
    void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};

void MessageViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItemV4 optionV4 = option;
    initStyleOption(&optionV4, index);
    QStyle *style = optionV4.widget? optionV4.widget->style() : QApplication::style();
    QTextDocument doc;
    QString align(index.data(MessageModel::TypeRole) < 2 ? "right" : "left");//changed 20180831
    QString margin_left(index.data(MessageModel::TypeRole) < 2 ? "margin-left:250;" :"margin-left:0;");
    QString margin_right(index.data(MessageModel::TypeRole) < 2 ? "margin-right:0;" :"margin-right:200;");
    QString background_color("");//QString background_color(index.data(MessageModel::UnreadFlagRole).toBool() ? "background-color:rgb(245, 245, 245);" : "");
    QString html;
    html = "<hr align=\"left\" width=\"100%\">";
    html += "<p align=\"" + align + "\" style=\""+ background_color + "; margin-top:10px; "
            +margin_left+margin_right+"margin-bottom:10px\">" + index.data(MessageModel::ShortMessageRole).toString() + "</p>";
    html += "<p align=\"" + align + "\" style=\"font-size:8px;"+background_color+margin_left+margin_right
            +"margin-top:1px; margin-bottom:1px\">" + index.data(MessageModel::ReceivedDateRole).toString() + "</p>";
    html += "<p align=\"" + align + "\" style=\"font-size:8px;"+background_color+margin_left+margin_right
            +"margin-top:1px; margin-bottom:1px\">" + index.data(MessageModel::LabelRole).toString() + " (";
    html += index.data(MessageModel::FromAddressRole).toString() + ")</p>";

    //QString unreadFlagTag = index.data(MessageModel::UnreadFlagRole).toBool() ? "background-color:rgb(245, 245, 245)" : "";

    doc.setHtml(html);

    // Painting item without text
    optionV4.text = QString();
    style->drawControl(QStyle::CE_ItemViewItem, &optionV4, painter);

    QAbstractTextDocumentLayout::PaintContext ctx;

    // Highlighting text if item is selected
    if (optionV4.state & QStyle::State_Selected)
        ctx.palette.setColor(QPalette::Text, optionV4.palette.color(QPalette::Active, QPalette::HighlightedText).lighter(190));

    //mouse over event test
//    if(optionV4.state & QStyle::State_MouseOver)
//        ctx.palette.setColor(QPalette::Text, optionV4.palette.color(QPalette::Active, QPalette::HighlightedText));
    /* test*/
//    painter->setPen( Qt::black );
//    QRect borderRect = QRect(optionV4.rect.topLeft()+10,optionV4.rect.topRight()+10,optionV4.rect.bottomLeft()-10,optionV4.rect.bottomRight()-10);
//    painter->drawRect(borderRect);

    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &optionV4);

    doc.setTextWidth( textRect.width() );
    painter->save();
    painter->translate(textRect.topLeft());
    painter->setClipRect(textRect.translated(-textRect.topLeft()));
    doc.documentLayout()->draw(painter, ctx);
    painter->restore();
}

QSize MessageViewDelegate::sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QStyleOptionViewItemV4 options = option;
    initStyleOption(&options, index);
    QTextDocument doc;
    doc.setHtml(index.data(MessageModel::HTMLRole).toString());
    doc.setTextWidth(options.rect.width());
    return QSize(doc.idealWidth(), doc.size().height()+20);
}


MessagePage::MessagePage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MessagePage),
    model(0),
    msgdelegate (new MessageViewDelegate())
    //messageTextEdit(new MRichTextEdit())
{
    ui->setupUi(this);
   // ui->sendButton->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Return));
   // Context menu actions
    replyAction           = new QAction(tr("Reply to sender"),            this);
//    copyFromAddressAction = new QAction(ui->copyFromAddressButton->text(), this);
//    copyToAddressAction   = new QAction(ui->copyToAddressButton->text(),   this);
//    deleteAction          = new QAction(ui->deleteButton->text(),          this);

    // Build context menu
    contextMenu = new QMenu();
    contextMenu->addAction(replyAction);
//    contextMenu->addAction(copyFromAddressAction);
//    contextMenu->addAction(copyToAddressAction);
//    contextMenu->addAction(deleteAction);

    connect(replyAction,           SIGNAL(triggered()), this, SLOT(on_replyToSenderOnly_clicked()));
//    connect(copyFromAddressAction, SIGNAL(triggered()), this, SLOT(on_copyFromAddressButton_clicked()));
//    connect(copyToAddressAction,   SIGNAL(triggered()), this, SLOT(on_copyToAddressButton_clicked()));
//    connect(deleteAction,          SIGNAL(triggered()), this, SLOT(on_deleteButton_clicked()));

    connect(ui->tableViewConversation, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextualMenu(QPoint)));

//    // Show Messages
    ui->tableViewConversation->setItemDelegate(msgdelegate);
    ui->tableViewConversation->setIconSize(QSize(DECORATION_SIZE, DECORATION_SIZE));
     ui->tableViewConversation->setMinimumHeight(NUM_ITEMS * (DECORATION_SIZE + 2));
     ui->tableViewConversation->setAttribute(Qt::WA_MacShowFocusRect, false);

//    ui->listConversation->setItemDelegate(msgdelegate);
//    ui->listConversation->setIconSize(QSize(DECORATION_SIZE, DECORATION_SIZE));
//    ui->listConversation->setMinimumHeight(NUM_ITEMS * (DECORATION_SIZE + 2));
//    ui->listConversation->setAttribute(Qt::WA_MacShowFocusRect, false);

//    ui->listConversation->setVisible(false);
//    ui->tableView->setVisible(false);
//    ui->listWidgetContacts->setVisible(false);

    //ui->listWidgetContacts->setVisible(false);
    //ui->listWidgetConversation->setVisible(false);

//    QAction* action = new QAction(d->ui.input);
//    action->setAutoRepeat(false);
//    action->setShortcut(tr("Ctrl+Return"));
//    connect(action, SIGNAL(triggered()), d->ui.eval, SLOT(click()));
//    d->ui.input->addAction(action);
    ui->plainTextEdit->installEventFilter(this);
    ui->treeWidget->setColumnWidth(0,140);

    ui->listWidgetConversation->setVisible(false);
    ui->bnEmoji->setVisible(false);
}

MessagePage::~MessagePage()
{
    delete ui;
}

void MessagePage::setModel(MessageModel *model)
{
    this->model = model;
    if(!model)
        return;

    //if (model->proxyModel)
    //    delete model->proxyModel;
    model->proxyModel = new QSortFilterProxyModel(this);
    model->proxyModel->setSourceModel(model);
    model->proxyModel->setDynamicSortFilter(true);
    model->proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    model->proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    model->proxyModel->sort(MessageModel::ReceivedDateTime);
    model->proxyModel->setFilterRole(MessageModel::Ambiguous);
    model->proxyModel->setFilterFixedString("true");

    model->proxyModelContacts = new QSortFilterProxyModel(this);
    model->proxyModelContacts ->setSourceModel(model);
    model->proxyModelContacts->setDynamicSortFilter(true);
    model->proxyModelContacts->setSortCaseSensitivity(Qt::CaseInsensitive);
    model->proxyModelContacts->setFilterCaseSensitivity(Qt::CaseInsensitive);
    model->proxyModelContacts->sort(MessageModel::ReceivedDateTime);
    model->proxyModelContacts->setFilterRole(MessageModel::Ambiguous);
    model->proxyModelContacts->setFilterFixedString("true");

    updateContactList();

    // Scroll to bottom, update views.
    connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(incomingMessage()));

    //signal if wallet unlocked
    connect (model, SIGNAL(walletUnlockedSignal()), this, SLOT(updateMessagePage()));

    //signal if join, leave channel or add o remove contact

    connect (model, SIGNAL(updateMessagePage()), this, SLOT(updateMessagePage()));


}

void MessagePage::on_sendButton_clicked()
{
    if(!model)
        return;

    if (ui->plainTextEdit->toPlainText().isEmpty())
        return;

   if (ui->treeWidget->selectedItems().size() == 0)
       return;

    std::string sError;
    std::string sendTo  = ui->treeWidget->currentItem()->text(1).toStdString();
    std::string message = ui->plainTextEdit->toPlainText().toStdString();
    std::string addFrom = ui->comboBox->currentText().toStdString();

    if (sendTo.empty() || message.empty() || addFrom.empty())
        return;

    if (SecureMsgSend(addFrom, sendTo, message, sError) != 0)
    {
        QMessageBox::warning(NULL, tr("Send Secure Message"),
            tr("Send failed: %1.").arg(sError.c_str()),
            QMessageBox::Ok, QMessageBox::Ok);

        return;
    };

    //ui->messageEdit->setMaximumHeight(30);
    ui->plainTextEdit->clear();
    ui->plainTextEdit->setFocus();
    ui->listWidgetConversation->scrollToBottom();
    ui->splitter_2->setSizes(QList<int>() << 100 << 200);

}

void MessagePage::on_newButton_clicked()
{
    if(!model)
        return;

    SendMessagesDialog dlg(SendMessagesDialog::Encrypted, SendMessagesDialog::Dialog, this);

    dlg.setModel(model);
    dlg.exec();
}

void MessagePage::on_copyFromAddressButton_clicked()
{
    GUIUtil::copyEntryData(ui->tableViewConversation, MessageModel::FromAddress, Qt::DisplayRole);
}

void MessagePage::on_copyToAddressButton_clicked()
{
    //GUIUtil::copyEntryData(ui->tableView, MessageModel::ToAddress, Qt::DisplayRole);
}

void MessagePage::on_deleteButton_clicked()
{
    QListView *list = ui->tableViewConversation;

    if(!list->selectionModel())
        return;

    QModelIndexList indexes = list->selectionModel()->selectedIndexes();

    if(!indexes.isEmpty())
    {
        list->model()->removeRow(indexes.at(0).row());
        indexes = list->selectionModel()->selectedIndexes();

        //        if(indexes.isEmpty())
        //            on_backButton_clicked();
    }
}

bool MessagePage::IsContactInTheList(QString qstrAddress)
{
    QList <QTreeWidgetItem *> items = ui->treeWidget->findItems(qstrAddress, Qt::MatchContains | Qt::MatchRecursive, 1);
    if (items.size() != 0)
        return true;
    else
        return false;
}

QTreeWidgetItem * MessagePage::addContactToAddressList(QString qstdGroup, QString qstrAddress)//qstdGroup must be "Contacts" or "Channels"
{
    //if  (IsContactInTheList()) return;
    QTreeWidget * contactTree =  ui->treeWidget;
    QTreeWidgetItem *contactsGroup;
    contactsGroup = contactTree->findItems(qstdGroup, Qt::MatchExactly, 0)[0];//we suppose only 1 group item can be found
    QTreeWidgetItem * childItem = new QTreeWidgetItem(contactsGroup);
    contactsGroup->addChild(childItem);
    QString associatedLabel = model->getWalletModel()->getAddressTableModel()->labelForAddress(qstrAddress);
    childItem->setText(0,associatedLabel);//TODO: add name from addressbook
    childItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    childItem->setText(1, qstrAddress);
    return childItem;
}

void MessagePage::on_replyToSenderOnly_clicked()
{
    //check if contact already in the list
    // else add contact to list
    //select it
    //focus to enter message
    QListView *list = ui->tableViewConversation;
    if(!list->selectionModel())
        return;
    QModelIndex index = list->selectionModel()->selectedIndexes().at(0);
    QString qstrSenderAddress;
//    if(!indexes.isEmpty())
//    {
        qstrSenderAddress = index.data(MessageModel::FromAddressRole).toString();
        //index.data(MessageModel::HTMLRole).toString()
//    }
    //check if qstrSenderAddress not empty
    if (!IsContactInTheList(qstrSenderAddress))
    {
        ui->treeWidget->setCurrentItem(addContactToAddressList(QString("Contacts"),qstrSenderAddress));
        ui->plainTextEdit->setFocus();
    }
    else
       ui->treeWidget->setCurrentItem(ui->treeWidget->findItems(qstrSenderAddress, Qt::MatchContains | Qt::MatchRecursive, 1)[0]);//not optmized line


}

void MessagePage::updateMessagePage()
{
    updateContactList();
}

void MessagePage::incomingMessage()
{
    _updateConversationListRequired = false;
    updateContactList();//update only if not found in  contact list tree
    _updateConversationListRequired = true;

    //find contact
    QModelIndex last_index = model->index(model->rowCount(QModelIndex())-1,0, QModelIndex());
    auto messageType = model->data(last_index, model->data (last_index,MessageModel::TypeRole).toInt());


    //try to skip message to youself

    if (messageType != 0 )//|| toAddressRole == fromAddressRole)
    {
        auto messageFrom = model->data(last_index, model->data (last_index,MessageModel::TypeRole) == 2 ?
                                           MessageModel::FromAddressRole : MessageModel::ToAddressRole);
        QTreeWidget * contactTree = ui->treeWidget;
        QList <QTreeWidgetItem *> items = contactTree->findItems(messageFrom.toString(), Qt::MatchContains | Qt::MatchRecursive, 1);
        if (items.size() != 0)
        {
            BOOST_FOREACH (QTreeWidgetItem *item, items)
            {
                QFont font;
                font.setBold(true);
                item->setFont(0,font);
                item->setFont(1,font);
            }

        }
    }
    //check is contact added


    //TODO: clean comments and relocate procedure below
    on_treeWidget_itemSelectionChanged();//update messages
}

void MessagePage::exportClicked()
{
    // CSV is currently the only supported format
    QString filename = GUIUtil::getSaveFileName(
            this,
            tr("Export Messages"), QString(),
            tr("Comma separated file (*.csv)"), NULL);

    if (filename.isNull()) return;

    CSVModelWriter writer(filename);

    // name, column, role
    writer.setModel(model->proxyModel);
    writer.addColumn("Type",             MessageModel::Type,             Qt::DisplayRole);
    writer.addColumn("Label",            MessageModel::Label,            Qt::DisplayRole);
    writer.addColumn("FromAddress",      MessageModel::FromAddress,      Qt::DisplayRole);
    writer.addColumn("ToAddress",        MessageModel::ToAddress,        Qt::DisplayRole);
    writer.addColumn("SentDateTime",     MessageModel::SentDateTime,     Qt::DisplayRole);
    writer.addColumn("ReceivedDateTime", MessageModel::ReceivedDateTime, Qt::DisplayRole);
    writer.addColumn("Message",          MessageModel::Message,          Qt::DisplayRole);

    if(!writer.write())
    {
        QMessageBox::critical(this, tr("Error exporting"), tr("Could not write to file %1.").arg(filename),
                              QMessageBox::Abort, QMessageBox::Abort);
    }
}

void MessagePage::contextualMenu(const QPoint &point)
{
   QModelIndex index = ui->tableViewConversation->indexAt(point);
    if(index.isValid())
    {
        contextMenu->exec(QCursor::pos());
    }
}

void MessagePage::on_listConversation_doubleClicked(const QModelIndex &index)
{
    //int nSelectedRow = index.row();
    //std::string strMessageText = ui->listConversation->item(nSelectedRow)->text().toStdString();
    QMessageBox::information(0, index.data(Qt::DisplayRole).toString()+" message", index.data(MessageModel::HTMLRole).toString());
}



void MessagePage::on_treeWidget_itemChanged(QTreeWidgetItem *item, int column)
{
    if (column != 0) return;
    std::string strAddress = item->text(1).toStdString();
    std::string strLabel = item->text(0).toStdString();
    if (!item->parent()) return;
    if (item->parent()->text(0) == "Contacts")
        model->UpdateAddressBook(strAddress, strLabel);
    else if (item->parent()->text(0) == tr("Channels"))
    {
        for (std::vector<SecMsgChannel>::iterator it = smsgChannels.begin(); it != smsgChannels.end(); ++it)
        {
            if (strAddress == it->sAddress)
            {
                it->sName = strLabel;
                break;
            }
        }

    };
//    if (!SecureMsgWriteIni()) //disabled to prevent too often i/o disk operations | TODO: check contact Tree is correct
//        return;
}

class ListConversationDelegate: public QAbstractItemDelegate
{

};

void MessagePage::on_treeWidget_itemSelectionChanged()
{
    //get last sent message to set up active account
    //if it's not exists, use received addres, if it's no channel (TODO!!)
    //if channel, use first config address

    this->model = model;
    if(!model)
        return;

    if (!_updateConversationListRequired)
        return;

    QComboBox * senderAddressComboBox = ui->comboBox;
    //QListWidget * conversationList = ui->listWidgetConversation;
    QTreeWidget * treeWidgetContactList = ui->treeWidget;

    QTreeWidgetItem *item = treeWidgetContactList->currentItem();
    if (!item)
        return;

    _selectedContactItem = item;
    QFont font;
    font.setBold(false);
    item->setFont(0,font);
    item->setFont(1,font);

    QListView * tableViewConversation = ui->tableViewConversation;
    auto proxyModelSelectedContactFilter = new QSortFilterProxyModel(this);
    proxyModelSelectedContactFilter->setSourceModel(model);
    QString strLastUsedAddress, strLastReceivedAddress;

    QString filter = item->text(1);
    proxyModelSelectedContactFilter->setFilterRole(false);
    proxyModelSelectedContactFilter->setFilterFixedString("");
    proxyModelSelectedContactFilter->sort(MessageModel::ReceivedDateTime);
    proxyModelSelectedContactFilter->setFilterRole(MessageModel::FilterAddressRole);
    proxyModelSelectedContactFilter->setFilterFixedString(filter);


    tableViewConversation->setItemDelegate(msgdelegate);
    tableViewConversation->setModel(proxyModelSelectedContactFilter);

    std::string addressTo;
    senderAddressComboBox->clear();
    for (std::vector<SecMsgAddress>::iterator it = smsgAddresses.begin(); it != smsgAddresses.end(); ++it)
    {
        CBitcoinAddress coinAddress(it->sAddress);
        addressTo = coinAddress.ToString();
        senderAddressComboBox->addItem(tr(addressTo.c_str()));
    }
    //Choose sender account (last used for selected contact item)
    //TODO: code below not tested.
    int nLastUsedAddressItemIndex;
    if (!strLastUsedAddress.isEmpty())
    {
        nLastUsedAddressItemIndex = senderAddressComboBox->findText(strLastUsedAddress);
        if (nLastUsedAddressItemIndex)
            senderAddressComboBox->setCurrentIndex(nLastUsedAddressItemIndex);
    }
    tableViewConversation->scrollToBottom();
    ui->plainTextEdit->setFocus();
}

void MessagePage::on_treeWidget_itemActivated(QTreeWidgetItem *item, int column)
{
    _selectedContactItem = item;

}

void MessagePage::updateContactList()
{
    this->model = model;
    if(!model)
        return;
    int numRows = model->rowCount(QModelIndex());
    int i;
    //Building Contact Tree Start
    QTreeWidget * contactTree =  ui->treeWidget;
    QString qstrSelectedAddress;

    QTreeWidgetItem * channelGroup;
    channelGroup = contactTree->findItems("Channels", Qt::MatchExactly, 0)[0];
    channelGroup->setExpanded(true);//TODO: restore last used value

    //Contact Group Item - first group

    QTreeWidgetItem *contactsGroup;
    contactsGroup = contactTree->findItems("Contacts", Qt::MatchExactly, 0)[0];
    contactsGroup->setExpanded(true);//TODO: restore last used value
    //Fill in  Subscribed Channels first
    BOOST_FOREACH(SecMsgChannel curChannel, smsgChannels)
    {
        QList <QTreeWidgetItem *> items = contactTree->findItems(QString::fromStdString(curChannel.sAddress), Qt::MatchContains | Qt::MatchRecursive, 1);
        if (items.size() != 0)
            continue;

        QTreeWidgetItem * childItem = new QTreeWidgetItem(channelGroup);
        channelGroup->addChild(childItem);
        childItem->setText(0,tr(GetChannelName(curChannel.sAddress).c_str()));
        childItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        childItem->setText(1, tr(curChannel.sAddress.c_str()));
        if (childItem->text(0).trimmed().isEmpty())
        {
            childItem->setText(0,childItem->text(1));
            QFont font;
            font.setItalic(true);
            childItem->setFont(0,font);
        }
    };
    //TODO: add 'Ignored" group ?
    for (i=0; i<numRows; i++)
    {
        QModelIndex cur_index = model->index(i,0, QModelIndex());
        QString qstrAddress;
        auto messageType = model->data(cur_index,MessageModel::TypeRole);
        if (model->data(cur_index,MessageModel::TypeRole) == 2) //Received msgs only uses 'from' field
            qstrAddress = model->data(cur_index,MessageModel::FromAddressRole).toString();
        else
            qstrAddress = model->data(cur_index,MessageModel::ToAddressRole).toString();
        QList <QTreeWidgetItem *> items = contactTree->findItems(qstrAddress, Qt::MatchContains | Qt::MatchRecursive, 1);
        if (items.size() != 0)
            continue;
        if (messageType == 0 || messageType == 2)
        {
            QTreeWidgetItem * childItem = new QTreeWidgetItem(contactsGroup);
            contactsGroup->addChild(childItem);
            QString associatedLabel = model->getWalletModel()->getAddressTableModel()->labelForAddress(qstrAddress);
            childItem->setText(0,associatedLabel);//TODO: add name from addressbook
            childItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            childItem->setText(1, qstrAddress);
        }
        else
        {
            //TODO: Cheick is it really needed
            QTreeWidgetItem * childItem = new QTreeWidgetItem(channelGroup);
            channelGroup->addChild(childItem);
            childItem->setText(0,tr(GetChannelName(qstrAddress.toStdString()).c_str()));
            childItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            childItem->setText(1, qstrAddress);
        };
    };
    //keep the item selected
    if (!qstrSelectedAddress.isEmpty())
    {
        QList <QTreeWidgetItem *> items = contactTree->findItems(qstrSelectedAddress, Qt::MatchContains | Qt::MatchRecursive, 1);
        if (items.size() != 0)
            contactTree->setCurrentItem(items[0]);//set first item.
    }
}

void MessagePage::updateMessageList()
{

}

bool MessagePage::eventFilter(QObject* obj, QEvent *event)
{
    if (obj != ui->plainTextEdit)
        return QWidget::eventFilter(obj, event);

    if(event->type() == QEvent::KeyPress) // Special key handling
    {
        QKeyEvent *keyevt = static_cast<QKeyEvent*>(event);
        int key = keyevt->key();
        Qt::KeyboardModifiers mod = keyevt->modifiers();
        switch(key)
        {
        case Qt::Key_Up:
            //if(obj == ui->lineEdit) { browseHistory(-1); return true; }
            break;
        case Qt::Key_Down:
            //if(obj == ui->lineEdit) { browseHistory(1); return true; }
            break;
        case Qt::Key_PageUp: /* pass paging keys to messages widget */
        case Qt::Key_PageDown:
            //if(obj == ui->lineEdit)
            //{
            //    QApplication::postEvent(ui->messagesWidget, new QKeyEvent(*keyevt));
            //    return true;
            //}
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            // forward these events
            if (!mod && obj == ui->plainTextEdit) {
                event->ignore();
                //sendMessage();
                ui->sendButton->animateClick();
                //QApplication::postEvent(ui->lineEdit, new QKeyEvent(*keyevt));
                return true;
            }
            break;
        default:
            // Typing in messages widget brings focus to line edit, and redirects key there
            // Exclude most combinations and keys that emit no text, except paste shortcuts
//            if(obj == ui->messagesWidget && (
//                  (!mod && !keyevt->text().isEmpty() && key != Qt::Key_Tab) ||
//                  ((mod & Qt::ControlModifier) && key == Qt::Key_V) ||
//                  ((mod & Qt::ShiftModifier) && key == Qt::Key_Insert)))
            {
//                ui->lineEdit->setFocus();
//                QApplication::postEvent(ui->lineEdit, new QKeyEvent(*keyevt));
                return false;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void MessagePage::on_bnEmoji_clicked()
{
    QWidget * emojiPanel = new QWidget();
    emojiPanel->show();

}

void MessagePage::on_pushButton_clicked()
{
   updateContactList();
}
