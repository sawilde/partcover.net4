Imports NUnit.Framework

<TestFixture()> _
Public Class TestClass

    <Test()> _
    Public Sub ExceptionFilterTest()
        For i As Integer = 1 To 100
            Try
                Throw New DemoException(i)
            Catch ex2 As DemoException When i Mod 2 = 0
                Debug.WriteLine("ex2")
            Catch ex As DemoException
                Debug.WriteLine("ex")
            End Try
        Next
    End Sub

End Class
