Imports NUnit.Framework

<TestFixture()> _
Public Class TestClass

    Public x As Integer

    <Test()> _
    Public Sub ExceptionTest()

        Try

        Catch ex As Exception ' When x = 0

        End Try

    End Sub

    <Test()> _
    Public Sub WhenTest()

        Try

        Catch ex As Exception When x = 0

        End Try

    End Sub

End Class
