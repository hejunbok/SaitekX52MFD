//#############################################################################
// Implementation der Klasse zum Speichern der Konfigurationsdaten des Plugins
//#############################################################################

#include "pluginConfigClass.hpp"
#include <regex>

using namespace std;
//using namespace std::tr1;


const std::tr1::regex debugName( "^debug$", std::tr1::regex_constants::icase );
const std::tr1::regex blinkName( "^blinkInterval$", std::tr1::regex_constants::icase );
const std::tr1::regex fireAName( "^fire_a$", std::tr1::regex_constants::icase );
const std::tr1::regex fireBName( "^fire_b$", std::tr1::regex_constants::icase );
const std::tr1::regex fireDName( "^fire_d$", std::tr1::regex_constants::icase );
const std::tr1::regex fireEName( "^fire_e$", std::tr1::regex_constants::icase );
const std::tr1::regex t1Name( "^t1$", std::tr1::regex_constants::icase );
const std::tr1::regex t2Name( "^t2$", std::tr1::regex_constants::icase );
const std::tr1::regex t3Name( "^t3$", std::tr1::regex_constants::icase );
const std::tr1::regex pov2Name( "^pov2$", std::tr1::regex_constants::icase );
const std::tr1::regex clutchName( "^clutch$", std::tr1::regex_constants::icase );
const std::tr1::regex fireButtonName( "^fire_button$", std::tr1::regex_constants::icase);
const std::tr1::regex throttleAxisName( "^throttle_axis$", std::tr1::regex_constants::icase );
// Special-LED's
// GEARS
const std::tr1::regex landingGearName( "^landing_gear_led$", std::tr1::regex_constants::icase );
const std::tr1::regex landingGearTrans("^landing_gear_trans$", std::tr1::regex_constants::icase );
const std::tr1::regex landingGearUp("^landing_gear_up$", std::tr1::regex_constants::icase );
const std::tr1::regex landingGearDown("^landing_gear_down$", std::tr1::regex_constants::icase );
const std::tr1::regex landingGearFail("^landing_gear_fail$", std::tr1::regex_constants::icase );
// FLAPS
const std::tr1::regex flapsStatusName( "^flaps_status_led$", std::tr1::regex_constants::icase );
const std::tr1::regex flapsStatusOut( "^flaps_status_out$", std::tr1::regex_constants::icase );
const std::tr1::regex flapsStatusUp( "^flaps_status_up$", std::tr1::regex_constants::icase );
const std::tr1::regex flapsStatusFull( "^flaps_status_full$", std::tr1::regex_constants::icase );
const std::tr1::regex flapsStatusFail( "^flaps_status_fail$", std::tr1::regex_constants::icase );
// SIM ist Running?
const std::tr1::regex isRunningStatusName( "^sim_is_running_led$", std::tr1::regex_constants::icase );
const std::tr1::regex isRunningStatusPaused( "^sim_is_paused$", std::tr1::regex_constants::icase );
const std::tr1::regex isRunningStatusRunning( "^sim_is_running$", std::tr1::regex_constants::icase );

// Values
const std::tr1::regex numberValue( "^[[:digit:]]+$" );
const std::tr1::regex boolValue( "true|1", std::tr1::regex_constants::icase );
const std::tr1::regex redValue( "red", std::tr1::regex_constants::icase );
const std::tr1::regex greenValue( "green", std::tr1::regex_constants::icase );
const std::tr1::regex amberValue( "amber", std::tr1::regex_constants::icase );
const std::tr1::regex offValue( "off|0|false", std::tr1::regex_constants::icase );
const std::tr1::regex blinkValue( "blink", std::tr1::regex_constants::icase );
const std::tr1::regex statusLedValue( "fire_a|fire_b|fire_d|fire_e|t1|t2|t3|pov2|clutch|fire_button", std::tr1::regex_constants::icase );
// etc
const std::tr1::regex commentLine("^#.*");                                             // Kommentarzele, ignorieren
const std::tr1::regex comment("#.*$");                                                 // Kommentar, l�schen
const std::tr1::regex emptyLine( "^\\s*$" );                                           // Leere Zeilen
const std::tr1::regex trimStrings( "(^\\s*)|(\\s*$)" );                                // Strings trimmen


//#############################################################################
// Standart Konstruktor
//#############################################################################
pluginConfigClass::pluginConfigClass(void):
landingGear( NONE_LED ),flapsStatus( NONE_LED )
{
// default nach Compiler-einstellung...
#ifdef _DEBUG
  debug = true;
#else
  debug = false;
#endif
}
//#############################################################################
// Konstruktor mit Angabe der Konfigurationsdatei
//#############################################################################
pluginConfigClass::pluginConfigClass( std::string& configFile ) : 
  blinkInterval(1000),debug(false),configFileName( configFile ),
  landingGear( NONE_LED ),flapsStatus( NONE_LED ),simIsRunning( NONE_LED )
{
}

//#############################################################################
// Destruktor
//#############################################################################
pluginConfigClass::~pluginConfigClass(void)
{
}

//#############################################################################
// Member zum Einf�gen der Referenz...
//#############################################################################
void pluginConfigClass::setLogThreadClass( logThreadClass* _logThreadClass )
{
  ltClass = _logThreadClass;
}

//#############################################################################
//Liest Config von Datei....
//#############################################################################
void pluginConfigClass::readConfig( void )
{
  string inString; 
  string pName;
  string pValue;
  fstream* configStream;

  initDefaults();                                                              // DEFAULTS machen

  if( configFileName.empty() )
  {
    // mache Defaults....
    if( debug ) logLine( "<pluginConfigClass::readConfig>: MAKE DEFAULTS...." );
    return;
  }

  try
  {
    if( debug ) logLine( string("<pluginConfigClass::readConfig>: READ FILE ") + configFileName );
    configStream = new fstream( configFileName.c_str(), ios_base::in );
    if( !configStream->is_open() )
    {
      errorLine( string("<pluginConfigClass::readConfig>: can't open ") + configFileName );
    }
    if( debug ) logLine( "<pluginConfigClass::readConfig>: READING..." );
    // gesamte Datei einlesen
    while( !configStream->eof() )
    {
      getline( *configStream, inString );                                      // lese die Zeile
      if( !computeLine( inString, pName, pValue ) )                            // Trenne die Parameter
        continue;                                                              // Fehlerhaft oder Leerzeile
      if( debug ) logLine( string("<pluginConfigClass::readConfig>: ") + pName + string("=") + pValue );                     // user informieren
      computeValuePair( pName, pValue );                                       // das Paar der Config zuordnen
    }
    configStream->close();                                                     // Schliesse Datei und Stream
    overrideSpecials();
    if( debug ) printConfigValues();                                           // CONFIG Summary
  }
  catch( char *_msg )
  {
    errorLine( _msg );                                                         // Fehler anzeigen
  }
}

//#############################################################################
// Konfiguration noch einmal in der Zusammenfasung
//#############################################################################
void pluginConfigClass::printConfigValues(void)
{
  logLine( string("<pluginConfigClass::printConfigValues>: Summary Config"));
  //FIRE_A, FIRE_B, FIRE_D, FIRE_E, T1, T2, T3, POV2, CLUTCH, FIRE_BUTTON, THROTTLE_AXIS
  logLine( string( "LED-DEFAULT-CONFIGS (may override from specials): ") );
  logLine( string( "FIRE_A        : ") + sayColor( ledStat[ FIRE_A ] ));
  logLine( string( "FIRE_B        : ") + sayColor( ledStat[ FIRE_B ] ));
  logLine( string( "FIRE_D        : ") + sayColor( ledStat[ FIRE_D ] ));
  logLine( string( "FIRE_E        : ") + sayColor( ledStat[ FIRE_E ] ));
  logLine( string( "T1            : ") + sayColor( ledStat[ T1 ] ));
  logLine( string( "T2            : ") + sayColor( ledStat[ T2 ] ));
  logLine( string( "T3            : ") + sayColor( ledStat[ T3 ] ));
  logLine( string( "POV2          : ") + sayColor( ledStat[ POV2 ] ));
  logLine( string( "CLUTCH        : ") + sayColor( ledStat[ CLUTCH ] ));
  logLine( string( "FIRE_BUTTON   : ") + sayColor( ledStat[ FIRE_BUTTON ] ));
  logLine( string( "THROTTLE_AXIS : ") + sayColor( ledStat[ THROTTLE_AXIS ] ));

  logLine( string( "SPECIAL-LED's (if defined):" ));
  if( landingGear != NONE_LED )
  {
    logLine( string( "Gears LED UP        :" ) + sayLED( landingGear ) + string( " ") + sayColor( landingGearLedStats[ SA_OK1 ] ));
    logLine( string( "Gears LED TRANSFER  :" ) + sayLED( landingGear ) + string( " ") + sayColor( landingGearLedStats[ SA_TRANS ] ));
    logLine( string( "Gears LED DOWN      :" ) + sayLED( landingGear ) + string( " ") + sayColor( landingGearLedStats[ SA_OK2 ] ));
    logLine( string( "Gears LED FAIL      :" ) + sayLED( landingGear ) + string( " ") + sayColor( landingGearLedStats[ SA_FAILED ] ));
  }

  if( flapsStatus != NONE_LED )
  {
    logLine( string( "Flaps LED UP        :" ) + sayLED( flapsStatus ) + string( " ") + sayColor( flapsLedStats[ SA_OK1 ] ));
    logLine( string( "Flaps LED OUT       :" ) + sayLED( flapsStatus ) + string( " ") + sayColor( flapsLedStats[ SA_TRANS ] ));
    logLine( string( "Flaps LED FULL      :" ) + sayLED( flapsStatus ) + string( " ") + sayColor( flapsLedStats[ SA_OK2 ] ));
    logLine( string( "Flaps LED FAIL      :" ) + sayLED( flapsStatus ) + string( " ") + sayColor( flapsLedStats[ SA_FAILED ] ));
  }
 
  if( simIsRunning != NONE_LED )
  {
    logLine( string( "Game is RUNNING     :" ) + sayLED( simIsRunning ) + string( " ") + sayColor( simIsRunningLedStats[ SA_OK1 ] ));
    logLine( string( "Game is PAUSING     :" ) + sayLED( simIsRunning ) + string( " ") + sayColor( simIsRunningLedStats[ SA_OK2 ] ));
  }
}

//#############################################################################
// Defaults einstellen
//#############################################################################
void pluginConfigClass::initDefaults(void)
{
  int i;

  // init LED-Stati
  for( i=static_cast<int>(NONE_LED); i < static_cast<int>(THROTTLE_AXIS); i++ )
  {
    ledStat[ static_cast<saLED>(i) ].first  = SA_OFF;
    ledStat[ static_cast<saLED>(i) ].second = SA_NOBLINK;
  }
  // init special-LED
  for( i=0; i < static_cast<int>(SA_FAILED); i++ )
  {
    // Fahrwerk
    landingGearLedStats[ static_cast<saSigStat>(i) ].first  = SA_OFF;
    landingGearLedStats[ static_cast<saSigStat>(i) ].second = SA_NOBLINK;
    // Klappen
    flapsLedStats[ static_cast<saSigStat>(i) ].first  = SA_OFF;
    flapsLedStats[ static_cast<saSigStat>(i) ].second = SA_NOBLINK;
    // SIM ist Running
    simIsRunningLedStats[ static_cast<saSigStat>(i) ].first  = SA_OFF;
    simIsRunningLedStats[ static_cast<saSigStat>(i) ].second = SA_NOBLINK;
  }
}

//#############################################################################
// Specials einstellen
//#############################################################################
void pluginConfigClass::overrideSpecials(void)
{
  if( landingGear != NONE_LED )                                           // exist Special-LED?
  {
    ledStat[ landingGear ] = landingGearLedStats[ SA_OK1 ];
  }
  if( flapsStatus != NONE_LED )                                           // Ist die aktiv?
  {
    ledStat[ flapsStatus ] = flapsLedStats[ SA_OK1 ];
  }
  if( simIsRunning )
  {
    ledStat[ simIsRunning ] = simIsRunningLedStats[ SA_OK1 ];
  }

}

//#############################################################################
// paare meiner Konfiguration zuordnen
//#############################################################################
bool pluginConfigClass::computeValuePair( string& pName, string& pValue )
{
  saLED led;

  led = getLEDFromString( pName );                                        // ist es eine LED?
  if( led != NONE_LED )                                                   // wenn das eine LED ist, dann...
  {
    (ledStat[ led ]).first = getColor( pValue );                          // setze den Status als Voreinstellung
    (ledStat[ led ]).second = SA_NOBLINK;
  }
  // DEBUG?
  else if( regex_match( pName, debugName ) )
  {
    if( regex_match( pValue, boolValue ) )                                 // true value?
      debug = true;
    else
      debug = false;
  }
  // Blink interval
  else if( regex_match( pName, blinkName ) )
  {
    if( regex_match( pValue, numberValue ) )
    {
      blinkInterval = atoi( pValue.c_str() );
    }
    else
    {
      logLine( string("<pluginConfigClass::computeValuePair>: ") + pName + string(" should be a integer not <") + pValue + string(">") );
      return(false);
    }
  }
  // GEAR LED?
  else if( regex_match( pName, landingGearName ) )
  {
    led = getLEDFromString( pValue );
    if( led != NONE_LED )
    {
      landingGear = led;
    }
    else
      errorLine( string("<pluginConfigClass::computeValuePair>: unknown LED in <") + pName + string(">" ) );
  }
  // GEARS UP?
  else if( regex_match( pName, landingGearUp ) )
  {
    landingGearLedStats[ SA_OK1 ].first = getColor( pValue );
    if( regex_search( pValue, blinkValue ))
      landingGearLedStats[ SA_OK1 ].second = SA_BLINK;
  }
  // GEARS TRANSFER?
  else if( regex_match( pName, landingGearTrans ) )
  {
    landingGearLedStats[ SA_TRANS ].first = getColor( pValue );
    if( regex_search( pValue, blinkValue ))
      landingGearLedStats[ SA_TRANS ].second = SA_BLINK;
  }
  // GEARS UP?
  else if( regex_match( pName, landingGearDown ) )
  {
    landingGearLedStats[ SA_OK2 ].first = getColor( pValue );
    if( regex_search( pValue, blinkValue ))
      landingGearLedStats[ SA_OK2 ].second = SA_BLINK;
  }
  // GEARS FAIL?
  else if( regex_match( pName, landingGearFail ) )
  {
    landingGearLedStats[ SA_FAILED ].first = getColor( pValue );
    if( regex_search( pValue, blinkValue ))
      landingGearLedStats[ SA_FAILED ].second = SA_BLINK;
  }
  // FLAPS LED?
  else if( regex_match( pName, flapsStatusName ) )
  {
    led = getLEDFromString( pValue );
    if( led != NONE_LED )
    {
      flapsStatus = led;
    }
    else
      errorLine( string("<pluginConfigClass::computeValuePair>: unknown LED in <") + pName + string(">" ) );
  }
  // FLAPS UP?
  else if( regex_match( pName, flapsStatusUp ) )
  {
    flapsLedStats[ SA_OK1 ].first = getColor( pValue );
    if( regex_search( pValue, blinkValue ))
      flapsLedStats[ SA_OK1 ].second = SA_BLINK;
  }
  // FLAPS OUT?
  else if( regex_match( pName, flapsStatusOut ) )
  {
    flapsLedStats[ SA_TRANS ].first = getColor( pValue );
    if( regex_search( pValue, blinkValue ))
      flapsLedStats[ SA_TRANS ].second = SA_BLINK;
  }
  // FLAPS FULL?
  else if( regex_match( pName, flapsStatusFull ) )
  {
    flapsLedStats[ SA_OK2 ].first = getColor( pValue );
    if( regex_search( pValue, blinkValue ))
      flapsLedStats[ SA_OK2 ].second = SA_BLINK;
  }
  // FLAPS FULL?
  else if( regex_match( pName, flapsStatusFail ) )
  {
    flapsLedStats[ SA_FAILED ].first = getColor( pValue );
    if( regex_search( pValue, blinkValue ))
      flapsLedStats[ SA_FAILED ].second = SA_BLINK;
  }
  // Game Is Running-LED?
  else if( regex_match( pName, isRunningStatusName ) )
  {
    led = getLEDFromString( pValue );
    if( led != NONE_LED )
    {
      simIsRunning = led;
    }
    else
      errorLine( string("<pluginConfigClass::computeValuePair>: unknown LED in <") + pName + string(">" ) );
  }
  // Game ist running??
  else if( regex_match( pName, isRunningStatusRunning ) )
  {
    simIsRunningLedStats[ SA_OK1 ].first = getColor( pValue );
    if( regex_search( pValue, blinkValue ))
      simIsRunningLedStats[ SA_OK1 ].second = SA_BLINK;
  }
  // Game ist running??
  else if( regex_match( pName, isRunningStatusPaused ) )
  {
    simIsRunningLedStats[ SA_OK2 ].first = getColor( pValue );
    if( regex_search( pValue, blinkValue ))
      simIsRunningLedStats[ SA_OK2 ].second = SA_BLINK;
  }
  // sonstiges...
  else
  {
    errorLine( string("<pluginConfigClass::computeValuePair>: unknown param <") + pName + string(">" ) );
    return( false );
  }
  return( true );  
}

//#############################################################################
// gib Farbwert oder keine Farbe zur�ck, wenn String gegeben
//#############################################################################
saColor pluginConfigClass::getColor( string& pValue )
{
  if( regex_search( pValue, offValue ) )
    return( SA_OFF );
  if( regex_search( pValue, redValue ) )
    return( SA_RED );
  if( regex_search( pValue, greenValue ) )
    return( SA_GREEN );
  if( regex_search( pValue, amberValue ) )
    return( SA_AMBER );
  errorLine( string("<pluginConfigClass::getColor> wrong parameter <") + pValue + string("> for LED Color") );  
  return( SA_OFF );
}

//#############################################################################
// Gibt String fuer Farbwert zurueck
//#############################################################################
std::string pluginConfigClass::sayColor( saLedStat& _color )
{
  string msg;
  string blink;

  switch( _color.first )
  {
    case SA_OFF:
      msg = "OFF, ";
      break;
    case SA_RED:
      msg = "RED, ";
      break;
    case SA_GREEN:
      msg = "GREEN, ";
      break;
    case SA_AMBER:
      msg = "AMBER, ";
      break;
    default:
      msg = "UNKNOWN, ";
      break;
  }

  if( _color.second == SA_NOBLINK )
    blink = "NO BLINK";
  else
    blink = "BLINKING";

  return( string( msg + blink ) );
}

//#############################################################################
// welche LED ist gmemeint?
//#############################################################################
saLED pluginConfigClass::getLEDFromString( std::string& pName )
{
  // Fire A?
  if( regex_match( pName, fireAName ) )
  {
    return( FIRE_A );
  }
  // Fire B?
  else if( regex_match( pName, fireBName ) )
  {
    return( FIRE_B );
  }
  // Fire D?
  else if( regex_match( pName, fireDName ) )
  {
    return( FIRE_D );
  }
  // Fire E?
  else if( regex_match( pName, fireEName ) )
  {
    return( FIRE_E );
  }
  // T1?
  else if( regex_match( pName, t1Name ) )
  {
    return( T1 );
  }
  // T2?
  else if( regex_match( pName, t2Name ) )
  {
    return( T2 );
  }
  // T3?
  else if( regex_match( pName, t3Name ) )
  {
    return( T3 );
  }
  // POV2?
  else if( regex_match( pName, pov2Name ) )
  {
    return( POV2 );
  }
  // CLUTCH?
  else if( regex_match( pName, clutchName ) )
  {
    return( CLUTCH );
  }
  // FIRE?
  else if( regex_match( pName, fireButtonName ) )
  {
    return( FIRE_BUTTON );
  }
  // TROTTLE AXIS?
  else if( regex_match( pName, throttleAxisName ) )
  {
    return( THROTTLE_AXIS );
  }
  return( NONE_LED );
}

//#############################################################################
// LED benennen
//#############################################################################
std::string pluginConfigClass::sayLED( saLED led )
{
  switch( led )
  {
    case NONE_LED:
      return( string("NONE_LED"));
    case FIRE_A:
      return( string("FIRE_A"));
    case FIRE_B:
      return( string("FIRE_B"));
    case FIRE_D:
      return( string("FIRE_D"));
    case FIRE_E:
      return( string("FIRE_E"));
    case T1:
      return( string("T1"));
    case T2:
      return( string("T2"));
    case T3:
      return( string("T3"));
    case POV2:
      return( string("POV2"));
    case CLUTCH:
      return( string("CLUTCH"));
    case FIRE_BUTTON:
      return( string("FIRE_BUTTON"));
    case THROTTLE_AXIS:
      return( string("THROTTLE_AXIS"));
    default:
      return( string("UNKNOWN!"));
  }
  return( string("UNKNOWN!"));
}

//#############################################################################
// bearbeite und trenne Input...
//#############################################################################
bool pluginConfigClass::computeLine( std::string& _inLine, std::string& _vName, std::string& _vValue )
{
  string tempString;
  int pos;

  if( regex_match( _inLine, commentLine) )                                     // Komplett-Kommentar?
  {
    logLine( "<pluginConfigClass::computeLine>: Found: Comment => ignore..." );
    return(false);
  }
  tempString = regex_replace( _inLine, comment, string("") );                  // Kommentare entfernen
  if( regex_match( tempString, emptyLine ) )                                   // Leerzeilen entfernen
  {
    logLine( "<pluginConfigClass::computeLine>: Found: empty line => ignore..." );
    return(false);
  }
  pos = tempString.find("=");                                                  // suche  char 
  if( pos == string::npos )
  {
    errorLine( "<pluginConfigClass::computeLine>: Not Found: Char equal => ignore..." );
    errorLine( string("<pluginConfigClass::computeLine>: Line was: <") + tempString + string(">") );
    return(false);
  }
  _vName = tempString.substr(0, pos);                                          // Name holen
  _vValue = tempString.substr( pos+1, tempString.length() - pos - 1 );
  tempString = regex_replace( _vName, trimStrings, string("") );               // Leerzeichen trimmen
  _vName = tempString;
  tempString = regex_replace( _vValue, trimStrings, string("") );              // Leerzeichen trimmen
  _vValue = tempString;
  return(true);
}

//#############################################################################
// Logline Helperfunktion
//#############################################################################
void pluginConfigClass::logLine( std::string& _msg )
{
  if( ltClass )
    ltClass->logLine( _msg );
}

//#############################################################################
// logLine Helperfunktion
//#############################################################################
void pluginConfigClass::logLine( char* _msg )
{
  if( ltClass )
    ltClass->logLine( _msg );
}

//#############################################################################
// Logline Helperfunktion
//#############################################################################
void pluginConfigClass::errorLine( std::string& _msg )
{
  if( ltClass )
    ltClass->errorLine( _msg );
}

//#############################################################################
// logLine Helperfunktion
//#############################################################################
void pluginConfigClass::errorLine( char* _msg )
{
  if( ltClass )
    ltClass->errorLine( _msg );
}
