#include "meteoeci_gui.h"
#include "ui_meteoeci_gui.h"
#include "sensor.h"
#include <chrono>
#include <ctime>
#include <sstream>
#include <QDebug>

MeteoECI_GUI::MeteoECI_GUI(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MeteoECI_GUI)
{
    ui->setupUi(this);

    qDebug()<<"Aplicación iniciada...";

    QString nombre;
    nombre.append("baseDeDatos1.sqlite");

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("nombre");

    if(db.open()){
        qDebug()<<"Se ha conectado a la base de datos.";
    }else{
        qDebug()<<"ERROR! No se ha conectado a la base de datos.";
    }

    _estado = 0;
    _tmr = new QTimer();
    _tmr->setInterval( 50 );
    connect( _tmr, SIGNAL(timeout()), this, SLOT(leerSensores()) );
    _tmr->start();

    // Preguntar fecha y hora al SO
    ActualizarFechaFromSO();

    std::cout << _fecha << std::endl;

    _itera = 0;

    _min_gps1 = _min_gps2 = _min_gps3 = _min_Tem1 = _min_Tem2 = _min_Viento1 = _min_Viento2 = _min_Preci = 0;
    _max_gps1 = _max_gps2 = _max_gps3 = _max_Tem1 = _max_Tem2 = _max_Viento1 = _max_Viento2 = _max_Preci = 0;
    _g1 = _g2 = _g3 = _T1 = _T2 = _V1 = _V2 = _P = 0;
    _prom_gps[0] = _prom_gps[1] = _prom_gps[2] = 0.;
    _prom_Tem[0] = _prom_Tem[1] = 0.;
    _prom_Viento[0] = _prom_Viento[1] = 0.;
    _prom_Preci = 0.;
    for(int x = 0;x < 12;x++){
           _gps1[x] = _gps2[x] = _gps3[x] = _Tem1[x] = _Tem2[x] = _Viento1[x] = _Viento2[x] = _Preci[x] = .0;
     }

//    _db_local = new Db_Local("sensores.db");

    actualizarGUI();

    crearTablaUsuarios();
}

MeteoECI_GUI::~MeteoECI_GUI(){
    delete ui;
    delete _tmr;
}

void MeteoECI_GUI::actualizarGUI(){
    ui->txt_Hr->setText( QString::number( _hr ) );
    ui->txt_Min->setText( QString::number( _min ));
    ui->txt_Lat->setText( QString::number( _g1 ) + " °");
    ui->txt_Lon->setText( QString::number( _g2 ) +" °");
    ui->txt_Alt->setText( QString::number( _g3 ) +" m");
    ui->txt_Vel->setText( QString::number( _V1 ) + " km/hr");
    ui->txt_Dir->setText( QString::number( _V2 ) +" °");
    ui->txt_Temp->setText(QString::number( _T1 ) +" °C");
    ui->txt_Hum->setText(QString::number( _T2 ) + " %");
    ui->txt_Preci->setText(QString::number( _P ) + " mm/día");
}

void MeteoECI_GUI::crearTablaUsuarios()
{
    QString consulta;
    consulta.append("CREATE TABLE IF NOT EXISTS usuarios("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "nombre VARCHAR(100),"
                    "apellido VARCHAR(100),"
                    "edad INTEGER NOT NULL,"
                    "clase INTEGER NOT NULL"
                    ");");
    QSqlQuery crear;
    crear.prepare(consulta);

    if(crear.exec()){
        qDebug()<<"La tabla USUARIOS existe o se ha creado correctamente.";
    }else{
        qDebug()<<"La tabla USUARIOS NO existe o NO se ha creado correctamente.";
        qDebug()<<"ERROR!" << crear.lastError();
    }
}

void MeteoECI_GUI::insertarUsuario()
{
    QString consulta;
    consulta.append("INSERT INTO usuarios("
                    "nombre,"
                    "apellido,"
                    "edad,"
                    "clase)"
                    "VALUES("
                    //"'"+ui->Nombre->text()+"'"
                    "'DATO',"
                    "22,"
                    "1985"
                    ");");
    QSqlQuery insertar;
    insertar.prepare(consulta);

    if(insertar.exec()){
        qDebug()<<"El USUARIO se ha insertado correctamente.";
    }else{
        qDebug()<<"El USUARIO NO se ha insertado correctamente.";
        qDebug()<<"ERROR!" << insertar.lastError();
    }
}

void MeteoECI_GUI::leerSensores(){

    // Leer los sensores
    _gps.actualizar();
    _prom_gps[0] += _gps.latitud();
    _prom_gps[1] += _gps.longitud();
    _prom_gps[2] += _gps.altura();

    _tem.actualizar();
    _prom_Tem[0] += _tem.temperatura();
    _prom_Tem[1] += _tem.humedad();

    _viento.actualizar();
    _prom_Viento[0] += _viento.velocidad();
    _prom_Viento[1] += _viento.direccion();

    _preci.actualizar();
    _prom_Preci += _preci.leerDato();

    // Contador de cada 5 seg. Un minuto son 12.
    _itera++;
    if( _itera == 12 ){
        _itera = 0;
        _min++;
        if( _min == 60 ){
            _min = 0;
            _hr++;
           if( _hr == 24) {
               _hr = 0;
               ActualizarFechaFromSO();
           }
        }
        // Calcular promedios de minuto

        _prom_gps[0] /= 12.;
        _prom_gps[1] /= 12.;
        _prom_gps[2] /= 12.;

        _prom_Tem[0] /= 12.;
        _prom_Tem[1] /= 12.;

        _prom_Viento[0] /= 12.;
        _prom_Viento[1] /= 12.;


        // Acumular datos
        _gps1[0] = _prom_gps[0];
        _gps2[0] = _gps.longitud();
        _gps3[0] = _gps.altura();
        _Tem1[0] = _tem.temperatura();
        _Tem2[0] = _tem.humedad();
        _Viento1[0] = _viento.velocidad();
        _Viento2[0] = _viento.direccion();
        _Preci[0] = _preci.leerDato();

        int z = 12;
        for(int y=0;y<12;y++){
        _gps1[z] = _gps1[z-1];
        _gps2[z] = _gps2[z-1];
        _gps3[z] = _gps3[z-1];
        _Tem1[z] = _Tem1[z-1];
        _Tem2[z] = _Tem2[z-1];
        _Viento1[z] = _Viento1[z-1];
        _Viento2[z] = _Viento2[z-1];
        _Preci[z] = _Preci[z-1];
        z = z-1;
        }

        _min_gps1 = _gps1[0];
        _min_gps2 = _gps2[0];
        _min_gps3 = _gps3[0];
        _min_Tem1 = _Tem1[0];
        _min_Tem2 = _Tem2[0];
        _min_Viento1 = _Viento1[0];
        _min_Viento2 = _Viento2[0];
        _min_Preci = _Preci[0];

        //Hallar min
        for(int w=0;w<12;w++){
            if(_min_gps1 > _gps1[w]){
                _min_gps1 = _gps1[w];
            }
            if(_min_gps2 > _gps2[w]){
                _min_gps2 = _gps2[w];
            }
            if(_min_gps3 > _gps3[w]){
                _min_gps3 = _gps3[w];
            }
            if(_min_Tem1 > _Tem1[w]){
                _min_Tem1 = _Tem1[w];
            }
            if(_min_Tem2 > _Tem2[w]){
                _min_Tem2 = _Tem2[w];
            }
            if(_min_Viento1 > _Viento1[w]){
                _min_Viento1 = _Viento1[w];
            }
            if(_min_Viento2 > _Viento2[w]){
                _min_Viento2 = _Viento2[w];
            }
            if(_min_Preci > _Preci[w]){
                _min_Preci = _Preci[w];
            }
        }

        _max_gps1 = _gps1[0];
        _max_gps3 = _gps2[0];
        _max_gps2 = _gps3[0];
        _max_Tem1 = _Tem1[0];
        _max_Tem2 = _Tem2[0];
        _max_Viento1 = _Viento1[0];
        _max_Viento2 = _Viento2[0];
        _max_Preci = _Preci[0];

        //Hallar max
        for(int v=0;v<12;v++){
            if(_max_gps1 < _gps1[v]){
                _max_gps1 = _gps1[v];
            }
            if(_max_gps2 > _gps2[v]){
                _max_gps2 = _gps2[v];
            }
            if(_max_gps3 > _gps3[v]){
                _max_gps3 = _gps3[v];
            }
            if(_max_Tem1 > _Tem1[v]){
                _max_Tem1 = _Tem1[v];
            }
            if(_max_Tem2 > _Tem2[v]){
                _max_Tem2 = _Tem2[v];
            }
            if(_max_Viento1 > _Viento1[v]){
                _max_Viento1 = _Viento1[v];
            }
            if(_max_Viento2 > _Viento2[v]){
                _max_Viento2 = _Viento2[v];
            }
            if(_max_Preci > _Preci[v]){
                _max_Preci = _Preci[v];
            }
        }

        // Estados
        switch(_estado){
            case 0:
                ui->estado->setText("Apagado");
                _g1 =_g2 =_g3 =_V1 =_V2 =_T1= _T2 = _P =0;
                break;
            case 1:
                ui->estado->setText("Promedio");
                _g1 = _prom_gps[0];
                _g2 = _prom_gps[1];
                _g3 = _prom_gps[2];
                _V1 = _prom_Viento[0];
                _V2 = _prom_Viento[1];
                _T1 = _prom_Tem[0];
                _T2 = _prom_Tem[1];
                _P =  _prom_Preci;
                break;
            case 2:
                ui->estado->setText("Mínimo");
                _g1 = _min_gps1;
                _g2 = _min_gps2;
                _g3 = _min_gps3;
                _V1 = _min_Viento1;
                _V2 = _min_Viento2;
                _T1 = _min_Tem1;
                _T2 = _min_Tem2;
                _P =  _min_Preci;
                break;
            case 3:
                ui->estado->setText("Máximo");
                _g1 = _max_gps1;
                _g2 = _max_gps2;
                _g3 = _max_gps3;
                _V1 = _max_Viento1;
                _V2 = _max_Viento2;
                _T1 = _max_Tem1;
                _T2 = _max_Tem2;
                _P =  _max_Preci;
                break;
            default:
                ui->estado->setText("Apagado");
                _g1 =_g2 =_g3 =_V1 =_V2 =_T1= _T2 = _P =0;
                break;
        }

        // Actualizar GUI
        actualizarGUI();
/*        if( _db_local->abrir_DB() ){
            std::stringstream hora;
            hora << _hr << ":" << _min;
            _db_local->guardarDatos(_fecha, hora.str(), _prom_gps, _prom_Tem, _prom_Viento, _prom_Preci );
            _db_local->cerrar_DB();
        }
*/
        // Acumuladores a 0
        //_min_gps1 = _min_gps2 = _min_gps3 = _min_Tem1 = _min_Tem2 = _min_Viento1 = _min_Viento2 = _min_Preci = 0;
        //_max_gps1 = _max_gps2 = _max_gps3 = _max_Tem1 = _max_Tem2 = _max_Viento1 = _max_Viento2 = _max_Preci = 0;
        _prom_gps[0] = _prom_gps[1] = _prom_gps[2] = 0.;
        _prom_Tem[0] = _prom_Tem[1] = 0.;
        _prom_Viento[0] = _prom_Viento[1] = 0.;
        _prom_Preci = 0.;
        //for(int x = 0;x < 12;x++){
        //       _gps1[x] = _gps2[x] = _gps3[x] = _Tem1[x] = _Tem2[x] = _Viento1[x] = _Viento2[x] = _Preci[x] = 0.;
        //}


    }
}

void MeteoECI_GUI::ActualizarFechaFromSO()
{
    time_t now = time(0);
    tm *ltm = localtime(&now);
    _hr = ltm->tm_hour;
    _min = ltm->tm_min;

    std::stringstream fecha;
    fecha << ltm->tm_mday << "/" << ltm->tm_mon + 1  << "/" << ltm->tm_year + 1900;
    _fecha = fecha.str();
}

void MeteoECI_GUI::on_cmd_Recuperar_clicked()
{
    std::vector<double> vectDatos;
 /*   _db_local->abrir_DB();
    _db_local->recuperarDatos( vectDatos );

    for (int i=0; i < vectDatos.size(); i++)
        std::cout << (double)vectDatos[i] << ", ";

    std::cout << std::endl;
    _db_local->cerrar_DB();
 */
}
void MeteoECI_GUI::on_Back_clicked()
{
        if(_estado==0){
            _estado = 0;
        }
        else{
        _estado=_estado-1;
        }


}


void MeteoECI_GUI::on_Next_clicked()
{
    if(_estado==3){
        _estado = 1;
    }
    else{
    _estado=_estado+1;
    }

}

void MeteoECI_GUI::on_Agregarusuario_clicked()
{
    insertarUsuario();
}
