
function aaa( a, b ) 
	Console.Log( a, b ) 
end

run_cpp_lua( 1234, 3456, 98765, "122" );

var a = new Test_Base;
var ddd = "sfasdfsdf"; 

a.GetThis = function (v)
{
    Console.Log( "call GetThis", v:x(), v:y(), ddd );
   return 3;
}

a.NewThisLua = function ()
{
    return self;
}

a.kkk().x(10);
Console.Log( a.x(), ddd )
a.kkk( new CVector2f )
a.kkk().x(1000)
a.x(100)
r = a.GetNumber( new CVector2f, 10 )
Console.Log( r );
Console.Log( r.x(), r.y() );
Console.Log( Test_Base.GetCppName( a.kkk() ) );
Console.Log( a.TestRet64( "asdfff", "9580" ) );

