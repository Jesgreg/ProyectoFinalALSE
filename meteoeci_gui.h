#ifndef METEOECI_GUI_H
#define METEOECI_GUI_H


#include <QMainWindow>
#include <QTimer>
#include "sensor.h"
//#include "db_local.h"
#include <string>
#include<QtSql/QSqlDatabase>
#include<QtSql/QSqlQuery>
#include<QtSql/QSqlError>

namespace Ui {
class MeteoECI_GUI;
}

class MeteoECI_GUI : public QMainWindow
{
    Q_OBJECT

public:
    explicit MeteoECI_GUI(QWidget *parent = 0);
    ~MeteoECI_GUI();
    void actualizarGUI();
    void crearTablaUsuarios();
    void insertarUsuario();

public slots:
    void leerSensores();

private slots:
    void on_cmd_Recuperar_clicked();

    void on_Back_clicked();

    void on_Next_clicked();

    void on_Agregarusuario_clicked();

private:
    void ActualizarFechaFromSO();
    Ui::MeteoECI_GUI *ui;
    QSqlDatabase db;
    QTimer*     _tmr;
    GPS         _gps;
    Temp_Hum    _tem;
    Viento      _viento;
    Sensor      _preci;
    int         _hr, _min, _itera, _estado;
    float       _g1, _g2, _g3, _T1, _T2, _V1, _V2, _P,
                _min_gps1, _min_gps2, _min_gps3, _min_Tem1, _min_Tem2, _min_Viento1, _min_Viento2, _min_Preci,
                _max_gps1, _max_gps2, _max_gps3, _max_Tem1, _max_Tem2, _max_Viento1, _max_Viento2, _max_Preci;
    double      _prom_gps[3], _prom_Tem[2], _prom_Viento[2], _prom_Preci, _gps1[12], _gps2[12],
                _gps3[12], _Tem1[12], _Tem2[12], _Viento1[12], _Viento2[12], _Preci[12];
//    Db_Local    *_db_local;
    std::string _fecha;
};

#endif // METEOECI_GUI_H
