
(function () 
{
    function Test(bResult, szMessage) 
    {
        console.log(szMessage, " ........ ", (bResult ? "OK" : "Failed"));
    }

    var CApplicationHandler = function () 
    {
        IApplicationHandler.call(this);
    }

    window.XScript.class(CApplicationHandler, null, IApplicationHandler);
    var __proto = CApplicationHandler.prototype;

    __proto.OnTestPureVirtual = function (
        e, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13) 
    {
        Test(e == 1234, "Test enum value to Script");

        Test(v0 == -123, "Test int8 value to Script");
        Test(v1 == -12345, "Test int16 value to Script");
        Test(v2 == -12345678, "Test int32 value to Script");
        Test(v3 == -1234567891011, "Test int64 value to Script");
        Test(v4 == -123456789, "Test long value to Script");

        Test(v5 == 123, "Test uint8 value to Script");
        Test(v6 == 12345, "Test uint16 value to Script");
        Test(v7 == 12345678, "Test uint32 value to Script");
        Test(v8 == 1234567891011, "Test uint64 value to Script");
        Test(v9 == 123456789, "Test unsigned long value to Script");

        Test(v10 == 1234567.0, "Test float value to Script");
        Test(v11 == 123456789101112.0, "Test double value to Script");

        Test(v12 == "abcdefg", "Test const char* value to Script");
        Test(v13 == "abcdefg", "Test const wchar_t* value to Script");
        return "OK"
    }

    __proto.OnTestNoParamPureVirtual = function()
    {
        return "OK"
    }

    __proto.TestBuffer = function(buffer)
    {
	    Test( buffer.value == 23456, "Test buffer to Script" );
	    return buffer;
    }

    g_handler = new CApplicationHandler();
    g_App = CApplication.GetInst();

    var address = new SAddress(3, 5);
    address.nIP = 123456789;
    address.nPort = 1234;

    var config = new SApplicationConfig;

    g_App.TestVirtualObjectPointer = function (Handler)
    {
        Test(g_handler == Handler, "Test object pointer to Script");
        return CApplication.prototype.TestVirtualObjectPointer.call( this, Handler );
    }

    g_App.TestVirtualObjectReference = function (Config)
    {
        Test(config == Config, "Test object reference to Script");
        return CApplication.prototype.TestVirtualObjectReference.call( this, Config );
    }

    g_App.TestVirtualObjectValue = function (Config)
    {
        Test(config != Config, "Test object value to Script");
        return CApplication.prototype.TestVirtualObjectValue.call( this, Config );
    }
    
    var buffer = {};
    buffer.value = 23456;

    window.StartApplication = function(name, id) 
    {
        config.SetName( name );
        config.nID = id;
	    config.Address = address;

        Test( config.Address.nIP == 123456789 && config.Address.nPort == 1234, "Test object value member" );
	    Test( g_App.TestCallObjectPointer(g_handler) == g_handler, "Test return obj pointer" );
	    Test( g_App.TestCallObjectReference(config) == config, "Test return obj reference" );
	    Test( g_App.TestCallObjectValue(config) != config, "Test return obj value " );
        Test( g_App.TestCallPOD(1234, -123, -12345, -12345678, -1234567891011, -123456789, 123, 12345,
            12345678, 1234567891011, 123456789, 1234567, 123456789101112, "abcdefg", "abcdefg") == "OK",
            "Test return string");
        Test( g_App.TestNoParamFunction() == "OK", "Test function without parameter");
	    Test( g_App.TestBuffer( buffer ) == buffer, "Test TestBuffer return" );
    }

    console.log("Test javascript loaded");
})();