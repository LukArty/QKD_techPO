#include <streamwork.h>
#include <mainwindow.h>
#include <QMessageBox>


StreamWork::StreamWork(){
}

StreamWork::StreamWork( hwe::Conserial *stand){
    stand_ = stand;
}
void StreamWork::LaserTest(){
    api::SLevelsResponse response;
    api::AdcResponse response_1;
    int Power = 0;
    float y1_, y2_;
    while (Power <= 100)
    {
        //установка мощности
        response_1 = stand_->SetLaserPower(Power);
        QThread::msleep(120);
        //снятие показателей
        response = stand_->GetSignalLevels();
        y1_ = response.signal_.h_;
        y2_ = response.signal_.v_;
        emit emitdate(y1_, y2_, Power);

        //увеличение мощности
        Power = Power + 2;
    }
    emit finished();
}

void StreamWork::InitByPD(){
    api::InitResponse response;
    response = stand_->InitByPD();
    int Power, h_, v_, h_max, v_max, error;
    float aHalf_, aQuart_, bHalf_, bQuart_;
    error = response.errorCode_;
    aHalf_ = response.startPlatesAngles_.aHalf_;
    aQuart_ = response.startPlatesAngles_.aQuart_;
    bHalf_ = response.startPlatesAngles_.bHalf_;
    bQuart_ = response.startPlatesAngles_.bQuart_;
    h_ = response.startLightNoises_.h_;
    v_ = response.startLightNoises_.v_;
    h_max =response.maxSignalLevels_.h_;
    v_max = response.maxSignalLevels_.v_;
    Power = response.maxLaserPower_;
    emit finished1 (aHalf_, aQuart_, bHalf_, bQuart_, h_, v_, h_max, v_max, Power, error);
}

/// @brief Метод для преобразования строк в лист
QStringList StreamWork:: ConvertingArray (QString str){

    QStringList list = str.split("",Qt::SkipEmptyParts);
    /*foreach(QString num, list)
        cout << num.toInt() << endl;*/

    return list;
}
/// @brief метод вероятностей
QString StreamWork::ElectionPD_v2(int PDH, int PDV, int yh_, int yv_, int MaxSig_h){

    QString bit = "";
    double val = rand(); //рандом числа
    double val_max = RAND_MAX;
    double val_de = val/val_max;
    double h_de = double(yh_)/double(MaxSig_h); //определение в каком интервале находиться сигнал
    if(yh_ < 200 && yv_< 200){
        return bit='X';
    }
    else{
        if( val_de <= h_de){
            if(PDH == 0 && PDV == 1){
                return bit='0';
            }
            else if(PDH == 1 && PDV == 0){
                return bit='1';
            }
            else {
                return bit='X';
            }
        }
        else{
            if(PDH == 0 && PDV == 1){
                return bit='1';
            }
            else if(PDH == 1 && PDV == 0){
                return bit='0';
            }
            else {
                return bit='X';
            }
        }
    }
}

/// @brief метод сравнений
QString StreamWork::ElectionPD(int PDH, int PDV, int yh_, int yv_){

    QString bit = "";
    if(yh_ < 200 && yv_< 200){
        return bit='X';
    }
    else{
        if(PDH == 0 && PDV == 1){
            if(yh_ > yv_){return bit='0';}
            else if(yh_ < yv_){return bit='1';}
            else if (yh_ == yv_){return bit='X';}
            else if (yh_ == 0 && yv_ == 0){return bit='X';}
            else {
                return bit='X';
            }
        }
        else if(PDH == 1 && PDV == 0){
            if(yh_ > yv_){return bit='1';}
            else if(yh_ < yv_){return bit='0';}
            else if (yh_ == yv_){return bit='X';}
            else if (yh_ == 0 && yv_ == 0){return bit='X';}
            else {
                return bit='X';
            }
        }
        else {
            return bit='X';
        }
    }
}
/// @brief выполнение протокола ВВ84
QStringList StreamWork:: Protocol (){

    QStringList bit, signalH, signalV;
    float aHalf_,aQuart_,bHalf_,bQuart_;
    int PDH_, PDV_, MaxSig_h;
    QStringList Combit;
    QStringList  CombitA;
    QStringList blankbit;
    QStringList keybit;
    int nerror=0;
    flag = true;

    //запуск таймера
    QElapsedTimer timer;
    timer.start();

    //проверка версии протокола
    api::versionFirmwareResponse response1;
    response1 = stand_->GetCurrentFirmwareVersion();
    if(response1.major_ == 1 && response1.minor_ == 0 && response1.micro_ == 0){
        MaxSig_h = (stand_->GetMaxSignalLevels().signal_.h_)*1.1; //для протокола 1.0.0
    }
    else{
        MaxSig_h = (stand_->GetInitParams().maxSignalLevels_.h_)*1.1; //для протокола 1.5 и 1.2
    }


    int yh_ = 0, yv_=0;
    api::SendMessageResponse response;
    double Power = stand_->GetLaserPower().adcResponse_;
    int i;
    for(i = 0; i<= AliceBit.size()-1;i++){
        if(flag == true){
            //определение параметров для Боба
            if(BobBasis[i]=='1'){
                bHalf_= bHalf_1;
                bQuart_= bQuart_1;
                if(BobBit[i]=='1'){
                    PDH_ = PH_11;
                    PDV_ = PV_11;}
                else{
                    PDH_ = PH_10;
                    PDV_ = PV_10;}
            }
            else{
                bHalf_= bHalf_0;
                bQuart_= bQuart_0;
                if(BobBit[i]=='1'){
                    PDH_ = PH_01;
                    PDV_ = PV_01;}
                else{
                    PDH_ = PH_00;
                    PDV_ = PV_00;}
            }

            //определение параметров для Алисы
            if(AliceBasis[i]=='1'){
                if(AliceBit[i]=='1'){
                    aHalf_= aHalf_11;
                    aQuart_= aQuart_11;}
                else{
                    aHalf_= aHalf_10;
                    aQuart_= aQuart_10;}
            }
            else{
                if(AliceBit[i]=='1'){
                    aHalf_= aHalf_01;
                    aQuart_= aQuart_01;}
                else{
                    aHalf_= aHalf_00;
                    aQuart_= aQuart_00;
                }
            }

            stand_->SetPlatesAngles({aHalf_, aQuart_, bHalf_, bQuart_});
            response = stand_->Sendmessage({aHalf_, aQuart_, bHalf_, bQuart_}, Power);
            //снятие показателей
            yh_ = response.currentSignalLevels_.h_;
            yv_ = response.currentSignalLevels_.v_;

            /* if(yh_ < 200 && yv_< 200){
                QMessageBox::critical(NULL,
                                      "Ошибка!",
                                      "Лазер не работает!",
                                      QMessageBox::Ok);
                break;
            }*/
            signalH << QString::number(yh_);
            signalV << QString::number(yv_);

            //выбор метода для определения бита
            if (ElectionPD_)
            {
                bit << ElectionPD_v2(PDH_, PDV_, yh_, yv_,MaxSig_h);
            }
            else
            {
                bit << ElectionPD(PDH_, PDV_, yh_, yv_);
            }
            //Сравнение базисов
            for(int j = 0; j<= bit.size()-1;j++){
                if(AliceBasis[j] == BobBasis[j]){
                    Combit << bit[j];
                    CombitA<< AliceBit[j];
                }
                else {Combit << "X"; CombitA<<"X";}
            }
            //чистая строка
            for(int a = 0; a<= Combit.size()-1;a++){
                if((Combit[a] == '1' || Combit[a] == '0') && Combit[a]==CombitA[a]){
                    blankbit << Combit[a];
                }
                else{blankbit << " ";}
            }
            //Вывод ключа
            for(int v = 0; v<= blankbit.size()-1;v++){
                if(blankbit[v] == '1' || blankbit[v] == '0'){
                    keybit << blankbit[v];
                }
                else{keybit << "";}
            }
            //Подсчет процента ошибок
            for(int j = 0; j<= Combit.size()-1;j++){
                if(CombitA[j] == Combit[j]){
                    nerror++;
                }
            }
            double error = ((double(Combit.size())-double(nerror))/double(Combit.size()))*100;

            emit emitdate(i,AliceBit.size(),bit,Combit,blankbit,keybit,error);
            QThread::msleep(200);

            nerror=0;
            Combit.clear();
            CombitA.clear();
            blankbit.clear();
            keybit.clear();
        }
        else break;
    }

    float end_time = timer.elapsed(); //остановка таймера
    float search_time = end_time /1000;//CLOCKS_PER_SEC
    float speed = ((i+1)*1000)/(float)search_time;
    BobBasis.clear();
    BobBit.clear();
    AliceBasis.clear();
    AliceBit.clear();
    emit emitdate(search_time,speed, signalH,signalV);

    emit finished ();
    return bit;
}

/// @brief протокол ВВ84 с Евой
QStringList StreamWork:: Protocol_Eva (){

    QFile fileOut("./EvaBit.txt");
    fileOut.open(QFile::Append | QFile::Text);
    QTextStream writeStream (&fileOut);
    int PDH_, PDV_, MaxSig_h;
    QStringList Combit;
    QStringList  CombitA;
    QStringList blankbit;
    QStringList keybit;
    int nerror=0;
    flag = true;

    //запуск таймера
    QElapsedTimer timer;
    timer.start();

    //проверка версии протокола
    api::versionFirmwareResponse response1;
    response1 = stand_->GetCurrentFirmwareVersion();
    if(response1.major_ == 1 && response1.minor_ == 0 && response1.micro_ == 0){
        MaxSig_h = (stand_->GetMaxSignalLevels().signal_.h_)*1.1; //для протокола 1.0.0
    }
    else{
        MaxSig_h = (stand_->GetInitParams().maxSignalLevels_.h_)*1.1; //для протокола 1.5 и 1.2
    }

    QStringList bit, bit_e, signalH_AE, signalV_AE, signalH_EB, signalV_EB;
    QString bit_eva =""; //Биты полученные Евой
    float aHalf_,aQuart_,bHalf_,bQuart_,eHalf_,eQuart_;
    int yh_ = 0, yv_=0;
    api::SendMessageResponse response;
    double Power = stand_->GetLaserPower().adcResponse_;
    for(int i = 0; i<= AliceBit.size()-1;i++) {
        if(flag == true){
            //определение параметров для Ева
            if(EvaBasis[i]=='1'){
                eHalf_= bHalf_1;
                eQuart_= bQuart_1;
            }
            else{
                eHalf_= bHalf_0;
                eQuart_= bQuart_0;
            }

            //определение параметров для Боба
            if(BobBasis[i]=='1'){
                bHalf_= bHalf_1;
                bQuart_= bQuart_1;
                if(BobBit[i]=='1'){
                    PDH_ = PH_11;
                    PDV_ = PV_11;}
                else{
                    PDH_ = PH_10;
                    PDV_ = PV_10;}
            }
            else{
                bHalf_= bHalf_0;
                bQuart_= bQuart_0;
                if(BobBit[i]=='1'){
                    PDH_ = PH_01;
                    PDV_ = PV_01;}
                else{
                    PDH_ = PH_00;
                    PDV_ = PV_00;}
            }

            //определение параметров для Алисы
            if(AliceBasis[i]=='1'){
                if(AliceBit[i]=='1'){
                    aHalf_= aHalf_11;
                    aQuart_= aQuart_11;}
                else{
                    aHalf_= aHalf_10;
                    aQuart_= aQuart_10;}
            }
            else{
                if(AliceBit[i]=='1'){
                    aHalf_= aHalf_01;
                    aQuart_= aQuart_01;}
                else{
                    aHalf_= aHalf_00;
                    aQuart_= aQuart_00;
                }
            }

            response = stand_->Sendmessage({aHalf_, aQuart_, eHalf_, eQuart_}, Power);
            //снятие показателей
            yh_ = response.currentSignalLevels_.h_;
            yv_ = response.currentSignalLevels_.v_;

            /* if(yh_ < 200 && yv_< 200){
                QMessageBox::critical(NULL,
                                      "Ошибка!",
                                      "Лазер не работает!",
                                      QMessageBox::Ok);
                break;
            }*/

            signalH_AE << QString::number(yh_);
            signalV_AE << QString::number(yv_);
            //выбор метода для определения бита
            if (ElectionPD_)
            {
                bit_eva = ElectionPD_v2(PDH_, PDV_, yh_, yv_,MaxSig_h);
                bit_e << ElectionPD_v2(PDH_, PDV_, yh_, yv_,MaxSig_h);
            }
            else
            {
                bit_eva = ElectionPD(PDH_, PDV_, yh_, yv_);
                bit_e << ElectionPD(PDH_, PDV_, yh_, yv_);
            }
            //определение параметров для Евы
            if(EvaBasis[i]=='1'){
                if(bit_eva =='1'){
                    eHalf_= aHalf_11;
                    eQuart_= aQuart_11;}
                else{
                    eHalf_= aHalf_10;
                    eQuart_= aQuart_10;}
            }
            else{
                if(bit_eva =='1'){
                    eHalf_= aHalf_01;
                    eQuart_= aQuart_01;}
                else{
                    eHalf_= aHalf_00;
                    eQuart_= aQuart_00;
                }
            }

            response = stand_->Sendmessage({eHalf_, eQuart_, bHalf_, bQuart_}, Power);
            //снятие показателей
            yh_ = response.currentSignalLevels_.h_;
            yv_ = response.currentSignalLevels_.v_;

            signalH_EB << QString::number(yh_);
            signalV_EB << QString::number(yv_);

            //выбор метода для определения бита
            if (ElectionPD_)
            {
                bit << ElectionPD_v2(PDH_, PDV_, yh_, yv_,MaxSig_h);
            }
            else
            {
                bit << ElectionPD(PDH_, PDV_, yh_, yv_);
            }
            //Сравнение базисов
            for(int j = 0; j<= bit.size()-1;j++){
                if(AliceBasis[j] == BobBasis[j]){
                    Combit << bit[j];
                    CombitA<< AliceBit[j];
                }
                else {Combit << "X"; CombitA<<"X";}
            }
            //чистая строка
            for(int a = 0; a<= Combit.size()-1;a++){
                if((Combit[a] == '1' || Combit[a] == '0') && Combit[a]==CombitA[a]){
                    blankbit << Combit[a];
                }
                else{blankbit << " ";}
            }
            //Вывод ключа
            for(int v = 0; v<= blankbit.size()-1;v++){
                if(blankbit[v] == '1' || blankbit[v] == '0'){
                    keybit << blankbit[v];
                }
                else{keybit << "";}
            }
            //Подсчет процента ошибок
            for(int j = 0; j<= Combit.size()-1;j++){
                if(CombitA[j] == Combit[j]){
                    nerror++;
                }
            }
            double error = ((double(Combit.size())-double(nerror))/double(Combit.size()))*100;

            emit emitdate(i,AliceBit.size(),bit,bit_e,Combit,blankbit,keybit,error);
            QThread::msleep(200);

            nerror=0;
            Combit.clear();
            CombitA.clear();
            blankbit.clear();
            keybit.clear();
        }
        else break;
    }


    float end_time = timer.elapsed(); //остановка таймера
    float search_time = end_time /1000;//CLOCKS_PER_SEC
    float speed = (AliceBasis.size()*1000)/(float)search_time;
    emit emitdate_eva(search_time,speed, signalH_AE,signalV_AE, signalH_EB, signalV_EB);

    emit finished ();
    return bit;
}


void StreamWork::on_SetLaserStateBut_clicked()
{

}


void StreamWork::on_file_selection_clicked()
{

}

